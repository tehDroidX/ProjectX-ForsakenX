#if GL == 4
#include "render_gl_shared.h"

/*
 * GL 4.6-native backend. Same architecture as render_gl3.c (VAO cache keyed by
 * buffer handles + CPU shadow copies for game-code vertex work), but built on
 * direct state access (GL 4.5): buffers, VAOs and textures are created and
 * updated without ever touching a bind point, and the GLSL 460 shaders carry
 * explicit attribute/uniform locations (FSK_* in render_gl_shared.h) so no
 * glGet*Location lookups exist at all.
 */

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

// FSLock* hands game code the buffer's persistent CPU shadow copy (cached RAM -
// the per-frame vertex work of InterpFrames / vertex lighting is 10-30x faster
// there than on a glMapBuffer write-combined pointer, see render_gl2.c for the
// full story). FSUnlock* uploads it with one DSA call - no bind point involved.
//
// Deliberately NOT using GL 4.4 persistent-coherent mapped buffers here: those
// hand back the same write-combined memory class that caused the original
// framerate collapse. The shadow copy + upload is the right shape for this
// engine's read-modify-write vertex access pattern.

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
	glNamedBufferSubData( (GLuint)(size_t) handle, 0, size, sh );
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

/* Draw render object - see render_gl3.c for the general notes. GL4 differences:
 * - the VAO's layout is recorded with DSA calls (no binds during setup)
 * - all locations are the fixed FSK_* values, no glGet*Location anywhere
 * - textures bind via glBindTextureUnit (sampler uses layout(binding=0))
 * - the vnormal side buffer is not wired up: the shaders never had a vnormal
 *   input (lighting is CPU-side per-vertex color), so the GL2/GL3 code that
 *   queried it always got -1 and skipped it anyway.
 */

bool draw_render_object( RENDEROBJECT *renderObject, int primitive_type, bool orthographic )
{
	TEXTUREGROUP *group;
	texture_t *texdata;
	int i;

	// VAO cache keyed by the (stable) buffer handles - NOT stored in the
	// RENDEROBJECT, which is copied by value for transparent objects (that
	// leaked a VAO every frame per projectile/effect). All copies of an object
	// share one cached VAO; it's evicted when the buffers are freed.
	{
		int vao_is_new;
		GLuint vao = vao_cache_get(
			(GLuint)(size_t) renderObject->lpVertexBuffer,
			(GLuint)(size_t) renderObject->lpNormalBuffer,
			(GLuint)(size_t) renderObject->lpIndexBuffer,
			orthographic, &vao_is_new );
		if ( vao_is_new )
		{
			GLuint vbuf = (GLuint)(size_t) renderObject->lpVertexBuffer;
			GLuint ibuf = (GLuint)(size_t) renderObject->lpIndexBuffer;
			GLsizei stride = orthographic ? sizeof(TLVERTEX) : sizeof(LVERTEX);

			glVertexArrayVertexBuffer( vao, 0, vbuf, 0, stride );
			if ( ibuf )
				glVertexArrayElementBuffer( vao, ibuf );

			if ( orthographic )
			{	// TLVERTEX: 4*float tlpos | COLOR | 2*float texcoords
				glEnableVertexArrayAttrib(  vao, FSK_ATTR_TLPOS );
				glVertexArrayAttribFormat(  vao, FSK_ATTR_TLPOS, 4, GL_FLOAT, GL_FALSE, 0 );
				glVertexArrayAttribBinding( vao, FSK_ATTR_TLPOS, 0 );
				glEnableVertexArrayAttrib(  vao, FSK_ATTR_VCOLOR );
				glVertexArrayAttribFormat(  vao, FSK_ATTR_VCOLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 16 );
				glVertexArrayAttribBinding( vao, FSK_ATTR_VCOLOR, 0 );
				glEnableVertexArrayAttrib(  vao, FSK_ATTR_VTEXC );
				glVertexArrayAttribFormat(  vao, FSK_ATTR_VTEXC, 2, GL_FLOAT, GL_FALSE, 20 );
				glVertexArrayAttribBinding( vao, FSK_ATTR_VTEXC, 0 );
			}
			else
			{	// LVERTEX: 3*float pos | COLOR | 2*float texcoords
				glEnableVertexArrayAttrib(  vao, FSK_ATTR_POS );
				glVertexArrayAttribFormat(  vao, FSK_ATTR_POS, 3, GL_FLOAT, GL_FALSE, 0 );
				glVertexArrayAttribBinding( vao, FSK_ATTR_POS, 0 );
				glEnableVertexArrayAttrib(  vao, FSK_ATTR_VCOLOR );
				glVertexArrayAttribFormat(  vao, FSK_ATTR_VCOLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 12 );
				glVertexArrayAttribBinding( vao, FSK_ATTR_VCOLOR, 0 );
				glEnableVertexArrayAttrib(  vao, FSK_ATTR_VTEXC );
				glVertexArrayAttribFormat(  vao, FSK_ATTR_VTEXC, 2, GL_FLOAT, GL_FALSE, 16 );
				glVertexArrayAttribBinding( vao, FSK_ATTR_VTEXC, 0 );
			}
		}
		glBindVertexArray( vao );
	}

	CHECK_GL_ERRORS;

	// Update and use the appropriate model/view/projection matrix
	if ( orthographic )
		ortho_update( current_program );
	else
		mvp_update( current_program );

	glUniform1i( FSK_U_ORTHO, orthographic ? GL_TRUE : GL_FALSE );

	for ( i = 0; i < renderObject->numTextureGroups; i++ )
	{
		group = &renderObject->textureGroups[i];
		glUniform1i( FSK_U_CK, group->colourkey ? GL_TRUE : GL_FALSE );
		if ( group->texture )
		{
			glUniform1i( FSK_U_TEX, GL_TRUE );
			texdata = (texture_t *) group->texture;
			glBindTextureUnit( 0, texdata->id );
		}
		else
			glUniform1i( FSK_U_TEX, GL_FALSE );
		glDrawElementsBaseVertex( primitive_type, group->numTriangles * 3, GL_UNSIGNED_SHORT, group->startIndex * sizeof(WORD), group->startVert );
	}

	CHECK_GL_ERRORS;

	// per-object VAO stays bound on exit (safe - see the note in render_gl2.c)

	return true;
}

#endif // GL == 4
