#if GL > 1
#include "gl2_load.h"
#include "main_sdl.h"   /* SDL_GL_GetProcAddress */
#include "util.h"       /* DebugPrintf */

PFNGLATTACHSHADERPROC glAttachShader = NULL;
PFNGLBINDBUFFERPROC glBindBuffer = NULL;
#if SDL_VERSION_ATLEAST(2,0,0)
/* forwarding wrapper - SDL2 prototypes glBlendColor, opengl32.lib doesn't export it */
static PFNGLBLENDCOLORPROC p_glBlendColor = NULL;
void APIENTRY glBlendColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
{
	if ( p_glBlendColor ) p_glBlendColor( red, green, blue, alpha );
}
#else
PFNGLBLENDCOLORPROC glBlendColor = NULL;
#endif
PFNGLBUFFERDATAPROC glBufferData = NULL;
PFNGLBUFFERSUBDATAPROC glBufferSubData = NULL;
PFNGLCOMPILESHADERPROC glCompileShader = NULL;
PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
PFNGLCREATESHADERPROC glCreateShader = NULL;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = NULL;
PFNGLDELETEPROGRAMPROC glDeleteProgram = NULL;
PFNGLDELETESHADERPROC glDeleteShader = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;
PFNGLGENBUFFERSPROC glGenBuffers = NULL;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation = NULL;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL;
PFNGLGETPROGRAMIVPROC glGetProgramiv = NULL;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL;
PFNGLGETSHADERIVPROC glGetShaderiv = NULL;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = NULL;
PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
PFNGLMAPBUFFERPROC glMapBuffer = NULL;
PFNGLSHADERSOURCEPROC glShaderSource = NULL;
PFNGLUNIFORM1IPROC glUniform1i = NULL;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = NULL;
PFNGLUNMAPBUFFERPROC glUnmapBuffer = NULL;
PFNGLUSEPROGRAMPROC glUseProgram = NULL;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;
FSKPFNGLGENERATEMIPMAPPROC glGenerateMipmap = NULL;
FSKPFNGLDRAWELEMENTSBASEVERTEXPROC glDrawElementsBaseVertex = NULL;
FSKPFNGLGENVERTEXARRAYSPROC glGenVertexArrays = NULL;
FSKPFNGLBINDVERTEXARRAYPROC glBindVertexArray = NULL;
FSKPFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = NULL;
FSKPFNGLGETSTRINGIPROC glGetStringi = NULL;

#if GL >= 4
PFNGLCREATEBUFFERSPROC glCreateBuffers = NULL;
PFNGLNAMEDBUFFERDATAPROC glNamedBufferData = NULL;
PFNGLNAMEDBUFFERSUBDATAPROC glNamedBufferSubData = NULL;
PFNGLCREATEVERTEXARRAYSPROC glCreateVertexArrays = NULL;
PFNGLVERTEXARRAYVERTEXBUFFERPROC glVertexArrayVertexBuffer = NULL;
PFNGLVERTEXARRAYELEMENTBUFFERPROC glVertexArrayElementBuffer = NULL;
PFNGLVERTEXARRAYATTRIBFORMATPROC glVertexArrayAttribFormat = NULL;
PFNGLVERTEXARRAYATTRIBBINDINGPROC glVertexArrayAttribBinding = NULL;
PFNGLENABLEVERTEXARRAYATTRIBPROC glEnableVertexArrayAttrib = NULL;
PFNGLBINDTEXTUREUNITPROC glBindTextureUnit = NULL;
PFNGLCREATETEXTURESPROC glCreateTextures = NULL;
PFNGLTEXTURESTORAGE2DPROC glTextureStorage2D = NULL;
PFNGLTEXTURESUBIMAGE2DPROC glTextureSubImage2D = NULL;
PFNGLTEXTUREPARAMETERFPROC glTextureParameterf = NULL;
PFNGLGENERATETEXTUREMIPMAPPROC glGenerateTextureMipmap = NULL;
PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback = NULL;
#endif

static int gl2_missing = 0;
static void *gl2_get(const char *name)
{
    void *p = SDL_GL_GetProcAddress(name);
    if (!p) { DebugPrintf("gl2_load: missing GL function %s\n", name); gl2_missing++; }
    return p;
}

void gl2_load_functions(void)
{
    gl2_missing = 0;
    glAttachShader = (PFNGLATTACHSHADERPROC) gl2_get("glAttachShader");
    glBindBuffer = (PFNGLBINDBUFFERPROC) gl2_get("glBindBuffer");
#if SDL_VERSION_ATLEAST(2,0,0)
    p_glBlendColor = (PFNGLBLENDCOLORPROC) gl2_get("glBlendColor");
#else
    glBlendColor = (PFNGLBLENDCOLORPROC) gl2_get("glBlendColor");
#endif
    glBufferData = (PFNGLBUFFERDATAPROC) gl2_get("glBufferData");
    glBufferSubData = (PFNGLBUFFERSUBDATAPROC) gl2_get("glBufferSubData");
    glCompileShader = (PFNGLCOMPILESHADERPROC) gl2_get("glCompileShader");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC) gl2_get("glCreateProgram");
    glCreateShader = (PFNGLCREATESHADERPROC) gl2_get("glCreateShader");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) gl2_get("glDeleteBuffers");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC) gl2_get("glDeleteProgram");
    glDeleteShader = (PFNGLDELETESHADERPROC) gl2_get("glDeleteShader");
    glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) gl2_get("glDisableVertexAttribArray");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) gl2_get("glEnableVertexAttribArray");
    glGenBuffers = (PFNGLGENBUFFERSPROC) gl2_get("glGenBuffers");
    glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) gl2_get("glGetAttribLocation");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) gl2_get("glGetProgramInfoLog");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC) gl2_get("glGetProgramiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) gl2_get("glGetShaderInfoLog");
    glGetShaderiv = (PFNGLGETSHADERIVPROC) gl2_get("glGetShaderiv");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) gl2_get("glGetUniformLocation");
    glLinkProgram = (PFNGLLINKPROGRAMPROC) gl2_get("glLinkProgram");
    glMapBuffer = (PFNGLMAPBUFFERPROC) gl2_get("glMapBuffer");
    glShaderSource = (PFNGLSHADERSOURCEPROC) gl2_get("glShaderSource");
    glUniform1i = (PFNGLUNIFORM1IPROC) gl2_get("glUniform1i");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) gl2_get("glUniformMatrix4fv");
    glUnmapBuffer = (PFNGLUNMAPBUFFERPROC) gl2_get("glUnmapBuffer");
    glUseProgram = (PFNGLUSEPROGRAMPROC) gl2_get("glUseProgram");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) gl2_get("glVertexAttribPointer");
    glGenerateMipmap = (FSKPFNGLGENERATEMIPMAPPROC) gl2_get("glGenerateMipmap");
    glDrawElementsBaseVertex = (FSKPFNGLDRAWELEMENTSBASEVERTEXPROC) gl2_get("glDrawElementsBaseVertex");
    glGenVertexArrays = (FSKPFNGLGENVERTEXARRAYSPROC) gl2_get("glGenVertexArrays");
    glBindVertexArray = (FSKPFNGLBINDVERTEXARRAYPROC) gl2_get("glBindVertexArray");
    glDeleteVertexArrays = (FSKPFNGLDELETEVERTEXARRAYSPROC) gl2_get("glDeleteVertexArrays");
    glGetStringi = (FSKPFNGLGETSTRINGIPROC) gl2_get("glGetStringi");
#if GL >= 4
    glCreateBuffers = (PFNGLCREATEBUFFERSPROC) gl2_get("glCreateBuffers");
    glNamedBufferData = (PFNGLNAMEDBUFFERDATAPROC) gl2_get("glNamedBufferData");
    glNamedBufferSubData = (PFNGLNAMEDBUFFERSUBDATAPROC) gl2_get("glNamedBufferSubData");
    glCreateVertexArrays = (PFNGLCREATEVERTEXARRAYSPROC) gl2_get("glCreateVertexArrays");
    glVertexArrayVertexBuffer = (PFNGLVERTEXARRAYVERTEXBUFFERPROC) gl2_get("glVertexArrayVertexBuffer");
    glVertexArrayElementBuffer = (PFNGLVERTEXARRAYELEMENTBUFFERPROC) gl2_get("glVertexArrayElementBuffer");
    glVertexArrayAttribFormat = (PFNGLVERTEXARRAYATTRIBFORMATPROC) gl2_get("glVertexArrayAttribFormat");
    glVertexArrayAttribBinding = (PFNGLVERTEXARRAYATTRIBBINDINGPROC) gl2_get("glVertexArrayAttribBinding");
    glEnableVertexArrayAttrib = (PFNGLENABLEVERTEXARRAYATTRIBPROC) gl2_get("glEnableVertexArrayAttrib");
    glBindTextureUnit = (PFNGLBINDTEXTUREUNITPROC) gl2_get("glBindTextureUnit");
    glCreateTextures = (PFNGLCREATETEXTURESPROC) gl2_get("glCreateTextures");
    glTextureStorage2D = (PFNGLTEXTURESTORAGE2DPROC) gl2_get("glTextureStorage2D");
    glTextureSubImage2D = (PFNGLTEXTURESUBIMAGE2DPROC) gl2_get("glTextureSubImage2D");
    glTextureParameterf = (PFNGLTEXTUREPARAMETERFPROC) gl2_get("glTextureParameterf");
    glGenerateTextureMipmap = (PFNGLGENERATETEXTUREMIPMAPPROC) gl2_get("glGenerateTextureMipmap");
    glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC) gl2_get("glDebugMessageCallback");
#endif
    DebugPrintf("gl2_load: loaded GL2+ functions (%d missing)\n", gl2_missing);
}
#endif /* GL > 1 */
