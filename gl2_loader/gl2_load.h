/* gl2_load - Windows GL2+ function loader for the Forsaken OpenGL port.
 * opengl32.lib only exports GL 1.1; everything >=2.0 must be fetched at
 * runtime via SDL_GL_GetProcAddress. The port started this (see the
 * commented-out bind_gl_funcs in render_gl_shared.c) but never finished the
 * Windows path. This completes it. Guarded by GL>1 so GL1 builds are untouched. */
#ifndef GL2_LOAD_H
#define GL2_LOAD_H
#if GL > 1
#include "SDL_version.h"  /* SDL_VERSION_ATLEAST */
#include "SDL_opengl.h"   /* PFN typedefs + GL enum constants */

/* typedefs absent from this SDL 1.2 glext snapshot */
typedef void (APIENTRYP FSKPFNGLGENERATEMIPMAPPROC)(GLenum target);
typedef void (APIENTRYP FSKPFNGLDRAWELEMENTSBASEVERTEXPROC)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);
typedef void (APIENTRYP FSKPFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint *arrays);
typedef void (APIENTRYP FSKPFNGLBINDVERTEXARRAYPROC)(GLuint array);
typedef void (APIENTRYP FSKPFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint *arrays);
typedef const GLubyte * (APIENTRYP FSKPFNGLGETSTRINGIPROC)(GLenum name, GLuint index);

extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLBINDBUFFERPROC glBindBuffer;
#if SDL_VERSION_ATLEAST(2,0,0)
/* SDL2's SDL_opengl.h already prototypes glBlendColor (GL 1.4 core). Redeclaring it as
   a function pointer here clashes (C2365). gl2_load.c instead defines a real forwarding
   glBlendColor() that calls the runtime-loaded pointer - matches the prototype, links
   the symbol that opengl32.lib doesn't export. Under SDL 1.2 (no prototype) keep the
   plain pointer as before. */
#else
extern PFNGLBLENDCOLORPROC glBlendColor;
#endif
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLMAPBUFFERPROC glMapBuffer;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
extern PFNGLUNMAPBUFFERPROC glUnmapBuffer;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern FSKPFNGLGENERATEMIPMAPPROC glGenerateMipmap;
extern FSKPFNGLDRAWELEMENTSBASEVERTEXPROC glDrawElementsBaseVertex;
extern FSKPFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern FSKPFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern FSKPFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
extern FSKPFNGLGETSTRINGIPROC glGetStringi;   /* GL3 core: opengl32.lib lacks it */

void gl2_load_functions(void);
#endif /* GL > 1 */
#endif /* GL2_LOAD_H */
