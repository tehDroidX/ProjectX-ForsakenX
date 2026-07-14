# ProjectX ForsakenX - modernised OpenGL/SDL client builds

<p align="center">
  <img src="docs/flygirl.gif" alt="FlyGirl above the fzk-cubes4 tribute map, GL 4.6 client" height="300">
  <a href="https://youtu.be/hcxFQ0sx3hU"><img src="https://img.youtube.com/vi/hcxFQ0sx3hU/maxresdefault.jpg" alt="fzk-cubes4 flythrough on the GL 4.6 client (YouTube)" height="300"></a>
</p>
<p align="center">In-game captures from the GL 4.6 DSA client on the community tribute map fzk-cubes4.<br>
The right image links to the full map flythrough on YouTube.</p>

This fork carries a chain of branches; each one is a reviewable step on top
of the previous one:

| branch | renderer | SDL | sound | note |
|---|---|---|---|---|
| `master` | (upstream) | | | unchanged ForsakenX/forsaken |
| `gl1-sdl1.2` | GL 1 fixed-function | 1.2 | OpenAL | one-line multiplayer-join OOM fix + standalone MSVC pipeline |
| `gl2-sdl1.2` | GL 2 (GLSL 120) | 1.2 | OpenAL | major performance fixes + in-game video mode changes |
| `gl3.2-sdl2.30` | GL 3.2 core (GLSL 150) | 2.30.9 | OpenAL | SDL2 port + input/sound/video-settings fixes |
| `gl4.6-sdl3.4` | GL 4.6 core context | 3.4.12 | OpenAL | SDL3 port, the recommended modern client |
| `gl4.6-dsa-sdl3.4` | GL 4.6 native (DSA, GLSL 460) | 3.4.12 | OpenAL | fully modern renderer path |

Every client remains 100% network- and data-compatible with the vanilla
1.18.2547 release: it joins stock servers and reads the stock data files.

Each branch is self-contained to build: the scripts under `build/preview`
derive all paths from their own location, auto-locate Visual Studio via
vswhere (or run them from an "x86 Native Tools Command Prompt"), and use only
the vendored headers and MSVC import libs in `build/preview/deps` plus the
SDL 1.2 compat headers in `build/msvc-smoke/compat`. The only prerequisite is
Visual Studio Build Tools with the "Desktop development with C++" workload
(x86). The prebuilt MinGW libraries from the separate ForsakenX/forsaken-libs
repository are NOT used here - their .a archives are not ABI-compatible with
MSVC, which is why the MSVC-native equivalents ship inside each branch.

What the build files do (every branch; on `gl1-sdl1.2` the pipeline runs with
`GL=1`, `patched/` holds only minimally fixed `stats.c`/`xmem.c`, and there is
no GL loader and no separate gl1t script pair):

| file | purpose |
|---|---|
| `build/preview/compile-client.bat` | compiles the ~100 engine sources listed in `obj/originals.rsp` plus the five maintained copies in `patched/` and the GL loader; objects go to `obj-client/`, compiler output to `cc-*.log` |
| `build/preview/link-client.bat` | links the branch's client exe from those objects and the import libs in `deps/lib`; linker output in `link-client.log` |
| `build/preview/compile-gl1t.bat` + `link-gl1t.bat` | the same pipeline with `GL=1`: builds a fixed-function `projectx_client_gl1t.exe` from this tree, i.e. this branch's fixes on the GL1 renderer (the `gl1-sdl1.2` release exe is built on that branch itself) |
| `build/preview/patched/` | five sources (`title.c`, `main.c`, `restart.c`, `stats.c`, `xmem.c`) compiled INSTEAD of their tree-root counterparts |
| `build/preview/deps/` | vendored headers (SDL, OpenAL, lua, enet, libpng, zlib) and MSVC import libs + runtime DLLs |
| `build/msvc-smoke/compat/` | SDL 1.2 and GL compat headers for MSVC |

Review tip: diff each branch against its parent, e.g.
`master...gl1-sdl1.2`, `gl1-sdl1.2...gl2-sdl1.2`, ... or
`master...gl4.6-dsa-sdl3.4` for the full picture.

The upstream readme is preserved as `README-upstream.md`.

---

## Branch `gl2-sdl1.2` - GL2 renderer made playable (GLSL 120 + SDL 1.2 + OpenAL)

Adds a self-contained MSVC build pipeline plus the fixes that take the
GL2 backend from ~20 fps to ~500 fps in combat.

### Build
* Visual Studio Build Tools (x86), then run
  `build\preview\compile-client.bat` and `build\preview\link-client.bat`;
  this produces `projectx_client_gl2.exe`. The scripts locate Visual
  Studio automatically and work from any checkout location.
* Copy the exe into a stock ProjectX 1.18.2547 install.
  All required import libs/headers ship in `build/preview/deps/`
  (MSVC-native .lib files - do NOT link MinGW .a archives, the ABI differs).
* `build/preview/deps/OpenAL32.dll` is openal-soft 1.25.2 (Win32), a
  recommended drop-in replacement for the very old DLL in the retail folder.

### Fixes in this branch (on top of gl1-sdl1.2)
1. **Windows GL2+ loader** (`gl2_loader/`): opengl32.lib only exports GL 1.1;
   every GL2+ entry point is fetched at runtime via `SDL_GL_GetProcAddress`.
2. **Ortho/2D pipeline**: the vertex shader's ortho branch was disabled
   (menus/HUD never drew); TLVERTEX.w is rhw, so 2D positions must be fed as
   `vec4(tlpos.xy, 0, 1)`; `near`/`far` renamed (windows.h macros).
3. **VAO cache keyed by GL buffer handles**: per-draw
   `glVertexAttribPointer` / `glEnable/DisableVertexAttribArray` (about nine
   calls per draw, ~15 us each on this context) capped the game at ~20 fps.
   A vertex array object records the layout once. The VAO must NOT live in
   the RENDEROBJECT struct: transexe.c queues transparent objects *by value*,
   which leaked one VAO per effect per frame (framerate melted as soon as
   projectiles flew). Keying the cache off the stable buffer handles makes
   all copies of an object share one VAO.
4. **CPU shadow buffers** (`shadow_*` in render_gl_shared.c): FSLock* used to
   hand game code the `glMapBuffer` pointer - write-combined, uncached
   memory. The per-frame vertex work (InterpFrames morph animation,
   per-vertex lighting, effect recolouring) runs 10-30x slower there;
   ModelDisp burned 30-100 ms per frame once effects piled up. FSLock* now
   returns a persistent malloc'd shadow copy (exactly what the GL1 backend
   does) and FSUnlock* uploads it with a single `glBufferSubData`. This is
   the rewrite the original comment in render_gl2.c asked for.
5. **mvp uniform location cached** per program (previously a string lookup
   per moving object).
6. **`CHECK_GL_ERRORS` compiled out by default** (define `CHECK_GL` to
   re-enable); calling glGetError several times per draw costs ~10x fps.
7. **In-game video mode changes with GL context loss recovery**: SDL 1.2 on
   Windows recreates the GL context on SDL_SetVideoMode, killing every
   texture/VBO/VAO/program - GL1 shrugs that off, GL2 used to crash, so both
   the fullscreen toggle and the resolution menu were dead on GL2. Now
   `gl_context_lost_reset()` drops the CPU-side caches (VAO cache, shadow
   copies) and the stale shader handles before the mode switch, and a context
   generation counter stamped into every render object stops releases of
   old-context objects from deleting same-numbered buffers that the reload
   has just created in the new context. After that the normal reinit path
   (shader rebuild in render_init, full texture/buffer reload via InitView)
   brings everything back - fullscreen toggle and resolution changes work
   in-game exactly like on GL1.
8. Debug helpers (opt-in via environment variables): `FSKPERF=1` logs an FPS
   line, `FSKDUMP=N` dumps the GL framebuffer to `gl2_dump_<0-5>.ppm` every
   N frames.
