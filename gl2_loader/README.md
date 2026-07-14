# gl2_loader - Windows GL2+ function loader for the Forsaken/ProjectX port

Self-contained, additive loader that makes the **GL=2** (and higher) builds link and run on
**Windows**, without needing SDL2. Drop-in, `#if GL > 1` guarded - **GL=1 builds are byte-for-byte
unaffected** (verified: the fork still compiles clean with `GL=1`).

## The problem it solves

Windows' `opengl32.lib` only exports **OpenGL 1.1**. Everything from GL 2.0 up
(`glCreateShader`, `glBindBuffer`, `glUseProgram`, `glUniform*`, `glVertexAttribPointer`, ...) must
be fetched at runtime via `wglGetProcAddress` / `SDL_GL_GetProcAddress`. On Linux/Mac these come
from libGL directly, so `GL=2`/`GL=3` link there - but on Windows the link fails with ~29
unresolved externals.

The port already started this (`render_gl_shared.c` has a commented-out `bind_gl_funcs` /
`bind_glBlendColor` under `#ifdef WIN32`) but never finished it - this completes it.

## What it does

- Declares the 29 GL2+ entry points the renderer uses as function pointers (using the `PFNGL...PROC`
  typedefs already present in the bundled `SDL_opengl.h`; two GL3.0/3.2 typedefs absent from that
  older glext snapshot - `glGenerateMipmap`, `glDrawElementsBaseVertex` - are declared here).
- `gl2_load_functions()` fetches each via `SDL_GL_GetProcAddress` and logs any that are missing.

## Integration (3 small hooks, all GL>1)

1. `render_gl_shared.h`: `#include "gl2_load.h"` (right after `#include "SDL_opengl.h"`).
2. `render_gl_shared.c`, `set_defaults()`: call `gl2_load_functions();` in the `#else` (GL>1)
   branch, just before `set_default_shaders()` (i.e. after the GL context exists, before any GL2
   call).
3. Build: add `-Igl2_loader`, compile `gl2_loader/gl2_load.c`.

Plus one unrelated **MSVC** fix in `render_gl_shared.c ortho_update()`: rename the locals
`near`/`far` -> `znear`/`zfar` (they are reserved macros from `windows.h`). Not needed on gcc.

## Verified (Windows, MSVC, SDL1, GL=2)

- Links cleanly (0 unresolved), starts, `gl2_load: loaded GL2+ functions (0 missing)`, shaders
  compile.
- **Renders correctly** (main menu + 3D scene - see `../gl2-render-proof.png`).
- Joins a dense map (fzk-cubes4, 40 pickups) with **flat memory** - GL2 uses `glDeleteBuffers`, so
  it does **not** have the GL1 `delete_buffer` leak that crashes the GL1 release client there.

## Note

This is deliberately minimal (only the functions the renderer actually calls). A production port
might prefer GLEW/glad, but this keeps the diff tiny and dependency-free. Contribution-ready for
upstream `ForsakenX/forsaken` as "Windows GL2/GL3 loader".
