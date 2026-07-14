#if GL == 3
#include "render_gl_shared.h"

bool FSCreateVertexBuffer(RENDEROBJECT *renderObject, int numVertices)
{
	renderObject->lpVertexBuffer = create_buffer( numVertices * sizeof(LVERTEX), GL_ARRAY_BUFFER, GL_STATIC_DRAW );
	return true;
}
bool FSCreateDynamicVertexBuffer(RENDEROBJECT *renderObject, int numVertices)
{
	renderObject->lpVertexBuffer = create_buffer( numVertices * sizeof(LVERTEX), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW );
	return true;
}

bool FSCreateNormalBuffer(RENDEROBJECT *renderObject, int numNormals)
{
	renderObject->lpNormalBuffer = create_buffer( numNormals * sizeof(NORMAL), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW );
	return true;
}
bool FSCreateDynamicNormalBuffer(RENDEROBJECT *renderObject, int numNormals)
{
	renderObject->lpNormalBuffer = create_buffer( numNormals * sizeof(NORMAL), GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW );
	return true;
}

bool FSCreateIndexBuffer(RENDEROBJECT *renderObject, int numIndices)
{
	renderObject->lpIndexBuffer = create_buffer( numIndices * 3 * sizeof(WORD), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW );
	return true;
}
bool FSCreateDynamicIndexBuffer(RENDEROBJECT *renderObject, int numIndices)
{
	renderObject->lpIndexBuffer = create_buffer( numIndices * 3 * sizeof(WORD), GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW );
	return true;
}

// Lock/Unlock used to glMapBuffer(GL_WRITE_ONLY) and hand game code the mapped
// pointer. That pointer is write-combined, uncached memory: the per-frame vertex
// work done between lock and unlock (InterpFrames morph animation, per-vertex
// dynamic lighting, effect recolouring) issues sparse reads/writes there and ran
// 10-30x slower than on the GL1 backend's plain malloc'd buffers - visible as the
// framerate collapsing as soon as projectiles/effects (= lit, animated, per-frame
// relocked models) piled up. Now FSLock* returns the buffer's persistent CPU
// shadow copy (cached RAM, exactly like GL1) and FSUnlock* uploads it with a
// single glBufferSubData - the rewrite the original port comment asked for.
//
// Uploads go through the GL_ARRAY_BUFFER target even for index buffers: buffer
// objects aren't typed, and this avoids touching GL_ELEMENT_ARRAY_BUFFER, which
// is recorded in whatever VAO happens to be bound.

static bool shadow_lock( void *handle, void **out, const char *who )
{
	*out = shadow_get( (GLuint)(size_t) handle, NULL );
	if ( !*out )
	{
		DebugPrintf( "%s: no shadow copy for buffer %u\n", who, (unsigned)(size_t) handle );
		return false;
	}
	return true;
}

static bool shadow_unlock( void *handle )
{
	int size = 0;
	void *sh = shadow_get( (GLuint)(size_t) handle, &size );
	if ( !sh )
		return false;
	glBindBuffer( GL_ARRAY_BUFFER, (GLuint)(size_t) handle );
	glBufferSubData( GL_ARRAY_BUFFER, 0, size, sh );
	CHECK_GL_ERRORS;
	return true;
}

bool FSLockVertexBuffer(RENDEROBJECT *renderObject, LVERTEX **verts)
{
	return shadow_lock( renderObject->lpVertexBuffer, (void **) verts, "FSLockVertexBuffer" );
}

bool FSUnlockVertexBuffer(RENDEROBJECT *renderObject)
{
	return shadow_unlock( renderObject->lpVertexBuffer );
}

bool FSLockNormalBuffer(RENDEROBJECT *renderObject, NORMAL **normals)
{
	return shadow_lock( renderObject->lpNormalBuffer, (void **) normals, "FSLockNormalBuffer" );
}

bool FSUnlockNormalBuffer(RENDEROBJECT *renderObject)
{
	return shadow_unlock( renderObject->lpNormalBuffer );
}

bool FSLockIndexBuffer(RENDEROBJECT *renderObject, WORD **indices)
{
	return shadow_lock( renderObject->lpIndexBuffer, (void **) indices, "FSLockIndexBuffer" );
}

bool FSUnlockIndexBuffer(RENDEROBJECT *renderObject)
{
	return shadow_unlock( renderObject->lpIndexBuffer );
}

bool FSCreateDynamic2dVertexBuffer(RENDEROBJECT *renderObject, int numVertices)
{
	renderObject->lpVertexBuffer = create_buffer( numVertices * sizeof(TLVERTEX), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW );
	return true;
}

bool FSLockPretransformedVertexBuffer(RENDEROBJECT *renderObject, TLVERTEX **verts)
{
	return FSLockVertexBuffer( renderObject, (LVERTEX **) verts );
}

/* Draw render object:
 * - if 2D (orthographic), set up appropriately:
 *   - orthographic projection matrix
 *   - ... plus scaling and translation for Y-flipping (T*S*P)
 *   else:
 *   - update mvp if necessary (mvp_needs_update)
 * - for each texture group (renderObject->numTextureGroups)
 *   - group = &renderObject->textureGroups[i]
 *   - if group->colourkey, enable color-keying
 *   - if group->texture, enable texturing and bind
 *     renderObject->textureGroups[group].texture
 *   - draw group->numVerts elements starting at group->startVert
 */

bool draw_render_object( RENDEROBJECT *renderObject, int primitive_type, bool orthographic )
{
	static const struct
	{
		const char *name;
		int components;
		GLenum type;
		GLboolean normalized;
		int offset;
	} normal_attr[] =
	{
		{ "pos",    3, GL_FLOAT,         GL_FALSE, 0  },
		{ "vcolor", 4, GL_UNSIGNED_BYTE, GL_TRUE,  12 }, // 3*float
		{ "vtexc",  2, GL_FLOAT,         GL_FALSE, 16 }, // 3*float + 1*COLOR
		{ NULL,     0, 0,                0,        0  }
	}, ortho_attr[] =
	{
		{ "tlpos",  4, GL_FLOAT,         GL_FALSE, 0 },
		{ "vcolor", 4, GL_UNSIGNED_BYTE, GL_TRUE,  16 }, // 4*float
		{ "vtexc",  2, GL_FLOAT,         GL_FALSE, 20 }, // 4*float + 1*COLOR
		{ NULL,     0, 0,                0,        0  }
	}, *attr;
	//GLuint current_program;
	TEXTUREGROUP *group;
	texture_t *texdata;
	int loc;
	int i;


	// Cache attribute/uniform locations once per program. render_gl3.c originally did a
	// glGet*Location string lookup PER attribute PER draw (plus per texture group) - a big
	// avoidable cost. current_program is stable, so query once and reuse.
	static GLuint cached_prog = (GLuint)-1;
	static GLint a_normal[4], a_ortho[4], a_vnormal, u_ortho_c, u_ck_c, u_tex_c;
	if ( cached_prog != current_program )
	{
		cached_prog = current_program;
		for ( i = 0; normal_attr[i].name; i++ ) a_normal[i] = glGetAttribLocation( current_program, normal_attr[i].name );
		for ( i = 0; ortho_attr[i].name;  i++ ) a_ortho[i]  = glGetAttribLocation( current_program, ortho_attr[i].name );
		a_vnormal = glGetAttribLocation( current_program, "vnormal" );
		u_ortho_c = glGetUniformLocation( current_program, "orthographic" );
		u_ck_c    = glGetUniformLocation( current_program, "colorkeying_enabled" );
		u_tex_c   = glGetUniformLocation( current_program, "texturing_enabled" );
	}

	// Vertex-format setup (glVertexAttribPointer + glEnable/DisableVertexAttribArray)
	// per draw was the dominant per-object cost. A vertex array object records the
	// attribute layout + buffer bindings ONCE; later frames just bind the VAO. The
	// VAO is keyed off the (stable) buffer handles in a cache (NOT stored in the
	// RENDEROBJECT, which is copied by value for transparent objects - that leaked a
	// VAO every frame per projectile/effect). All copies of an object share one
	// cached VAO; it's evicted when the buffers are freed.
	{
		int vao_is_new;
		GLuint vao = vao_cache_get(
			(GLuint)(size_t) renderObject->lpVertexBuffer,
			(GLuint)(size_t) renderObject->lpNormalBuffer,
			(GLuint)(size_t) renderObject->lpIndexBuffer,
			orthographic, &vao_is_new );
		glBindVertexArray( vao );
		if ( vao_is_new )
		{
			glBindBuffer( GL_ARRAY_BUFFER, renderObject->lpVertexBuffer );
			if ( renderObject->lpIndexBuffer )
				glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, renderObject->lpIndexBuffer );
			else
				glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

			// see the LVERTEX and TLVERTEX definitions inside include/new3d.h
			attr = orthographic ? ortho_attr : normal_attr;
			for ( i=0; attr[i].name; i++ )
			{
				loc = ( orthographic ? a_ortho : a_normal )[i];
				if (loc >= 0)
				{
					glVertexAttribPointer(
						loc,
						attr[i].components,
						attr[i].type,
						attr[i].normalized,
						orthographic ? sizeof(TLVERTEX) : sizeof(LVERTEX),
						attr[i].offset
					);
					glEnableVertexAttribArray( loc );
				}
			}

			if ( renderObject->lpNormalBuffer )
			{
				glBindBuffer( GL_ARRAY_BUFFER, renderObject->lpNormalBuffer );
				loc = a_vnormal;
				if (loc >= 0)
				{
					glVertexAttribPointer( loc, 3, GL_FLOAT, GL_FALSE, sizeof(NORMAL), 0 );
					glEnableVertexAttribArray( loc );
				}
			}
		}
	}

	CHECK_GL_ERRORS;

	// Update and use the appropriate model/view/projection matrix
	if ( orthographic )
		ortho_update( current_program );
	else
		mvp_update( current_program );

	if ( u_ortho_c >= 0 )
		glUniform1i( u_ortho_c, orthographic ? GL_TRUE : GL_FALSE );

	for ( i = 0; i < renderObject->numTextureGroups; i++ )
	{
		group = &renderObject->textureGroups[i];
		if ( u_ck_c >= 0 )
			glUniform1i( u_ck_c, group->colourkey ? GL_TRUE : GL_FALSE );
		if ( group->texture )
		{
			if ( u_tex_c >= 0 )
				glUniform1i( u_tex_c, GL_TRUE );
			texdata = (texture_t *) group->texture;
			glBindTexture( GL_TEXTURE_2D, texdata->id );
		}
		else if ( u_tex_c >= 0 )
			glUniform1i( u_tex_c, GL_FALSE );
		glDrawElementsBaseVertex( primitive_type, group->numTriangles * 3, GL_UNSIGNED_SHORT, group->startIndex * sizeof(WORD), group->startVert );
	}

	CHECK_GL_ERRORS;

	// per-object VAO stays bound on exit (safe - see the note in render_gl2.c)

	return true;
}

#endif // GL == 3
