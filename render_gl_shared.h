/*
  This file is shared internally by render_* family.
  For public facing header please use render.h
*/
#ifdef GL
#ifndef RENDER_GL_SHARED_INCLUDED
#define RENDER_GL_SHARED_INCLUDED

#include "main.h"
#include "util.h"
#include "render.h"
#include "texture.h"
#include "file.h"
#include <stdio.h>
#include "main_sdl.h"
#include "SDL_opengl.h"
#include "gl2_load.h"

extern render_info_t render_info;

extern GLenum render_last_gl_error;

// TODO invalid pointer
// Under SDL2 (GL3 build) we drop the GLU dependency entirely and map GL error codes
// to strings inline - avoids linking glu32 with mismatched calling convention.
#if SDL_VERSION_ATLEAST(2,0,0)
#define gluErrorString(e)\
	(e == 0x0500 ? "invalid enumerant" : \
	(e == 0x0501 ? "invalid value" : \
	(e == 0x0502 ? "invalid operation" : \
	(e == 0x0503 ? "stack overflow" : \
	(e == 0x0504 ? "stack underflow" : \
	(e == 0x0505 ? "out of memory" : \
	(e == 0x0506 ? "invalid framebuffer operation" : \
	(e == 0x8031 ? "table too large" : \
	 "unknown" \
	))))))))
#endif

const char * render_error_description( int e );

/* glGetError() forces a driver pipeline sync; calling it several times per draw
 * call tanks the framerate (~10x on GL2). Only run the checks when CHECK_GL is
 * defined at build time - otherwise this is a no-op. */
#ifdef CHECK_GL
#define CHECK_GL_ERRORS \
	do \
	{ \
		GLenum e; \
		while( ( e = glGetError() ) != GL_NO_ERROR ) \
		{ \
			render_last_gl_error = e; \
			DebugPrintf( "GL error: %s (%s:%d)\n", \
				gluErrorString(e),  __FILE__, __LINE__ ); \
		} \
	} while (0)
#else
#define CHECK_GL_ERRORS do {} while (0)
#endif


typedef struct { float anisotropic; } gl_caps_t;
extern gl_caps_t caps;

typedef struct { GLuint id; } texture_t; // Possibly later: GLuint bump_id;

//
// d3d stored the world/view matrixes
// and then multiplied them together before rendering
// in the following order: world * view * projection
// opengl handles only world and projection
// so we must emulate the behavior of world*view
// although we multiply the arguments backwards view*world
//

extern MATRIX proj_matrix;
extern MATRIX view_matrix;
extern MATRIX world_matrix;

#if GL != 1

void mvp_update( GLuint current_program );

extern GLuint vertex_shader;
extern GLuint fragment_shader;
extern GLuint current_program;

LPVERTEXBUFFER _create_buffer( int size, GLenum type, GLenum gettype, GLenum usage );

#define create_buffer( size, type, usage ) \
        _create_buffer( size, type, type ## _BINDING, usage )

/* VAO cache keyed by GL buffer handles - see render_gl_shared.c. The RENDEROBJECT
   struct is copied by value (transexe.c) so a per-struct VAO leaked every frame;
   keying off the stable buffer handles makes all copies share one VAO. */
GLuint vao_cache_get( GLuint vbuf, GLuint nbuf, GLuint ibuf, int ortho, int *is_new );
void   vao_cache_evict( GLuint vbuf, GLuint nbuf, GLuint ibuf );

/* CPU shadow copy per GL buffer - see render_gl_shared.c. FSLock* hands game code
   this malloc'd pointer (cached RAM, like the GL1 backend) instead of a glMapBuffer
   pointer (write-combined uncached memory, where the per-frame vertex work of
   InterpFrames / vertex lighting was 10-30x slower); FSUnlock* uploads it with one
   glBufferSubData. This is the rewrite the original port comment asked for. */
void * shadow_create( GLuint id, int size );
void * shadow_get( GLuint id, int * size );
void   shadow_free( GLuint id );

#if GL >= 4
/* Fixed attribute/uniform locations, matching the layout() qualifiers in the
   GLSL 460 shaders (render_gl_shared.c). With explicit locations every
   glGetAttribLocation/glGetUniformLocation lookup disappears. */
#define FSK_ATTR_POS      0
#define FSK_ATTR_TLPOS    1
#define FSK_ATTR_VCOLOR   2
#define FSK_ATTR_VTEXC    3
#define FSK_U_MVP         0
#define FSK_U_ORTHO_PROJ  1
#define FSK_U_ORTHO       2
#define FSK_U_CK          3
#define FSK_U_TEX         4
#endif

#endif // GL != 1

void FSReleaseRenderObject(RENDEROBJECT *renderObject);

#endif // RENDER_GL_SHARED_INCLUDED
#endif // GL ENABLED
