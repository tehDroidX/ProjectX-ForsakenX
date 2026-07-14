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

## Branch `gl4.6-sdl3.4` - SDL 3.4.12 + GL 4.6 core context

Port of the SDL2 build to current SDL3. The renderer is unchanged (the
GLSL 150 shaders are valid in any core context >= 3.2); the context is
requested as GL 4.6 core with an automatic 3.2 fallback.

### Build
`compile-client.bat` then `link-client.bat` produce
`projectx_client_sdl3.exe`. Ship `SDL3.dll` next to the exe. Note that there
is no SDL3main.lib anymore - the classic `main()` links with
`/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup`.

### SDL2 -> SDL3 changes (complete list)
* headers: `/I deps/include/SDL3 /I deps/include` so `<SDL.h>` resolves
* `SDL_Init` returns bool; the SDL_version struct is replaced by packed ints
* `SDL_CreateWindow(title, w, h, flags)` (no position args); WINDOW_SHOWN gone
* events renamed `SDL_EVENT_*`; window events are a top-level type range
  (`SDL_EVENT_WINDOW_FIRST..LAST`) instead of one event with a sub-type
* `SDL_KeyboardEvent.keysym.sym/mod` -> `.key/.mod`; `KMOD_*` -> `SDL_KMOD_*`
* letter keycodes renamed `SDLK_a..z` -> `SDLK_A..Z`, BACKQUOTE -> GRAVE
* `SDL_GetKeyboardState` returns `const bool*`
* cursor: `SDL_ShowCursor()/SDL_HideCursor()`; grab: `SDL_SetWindowMouseGrab`;
  relative mouse: `SDL_SetWindowRelativeMouseMode(window, bool)`
* fullscreen: `SDL_SetWindowFullscreen(window, bool)`; exclusive modes for
  concrete resolution picks via `SDL_GetClosestFullscreenDisplayMode` +
  `SDL_SetWindowFullscreenMode` (NULL = borderless desktop for the "default"
  entry), windowed resizes via `SDL_SetWindowSize`, settled with
  `SDL_SyncWindow`; `SDL_GL_GetDrawableSize` -> `SDL_GetWindowSizeInPixels`
* audio: `SDL_LoadWAV(path, spec, buf, &len)` returns bool, SDL_AudioSpec has
  no `.size` field, there is no unsigned-16 format, `SDL_FreeWAV` -> `SDL_free`
* `SDL_StartTextInput/StopTextInput` take the window
* renderer flags removed (we drive our own GL context anyway)
* joystick: SDL3 addresses sticks by instance id; the port's state arrays
  index raw ids, so joystick support is disabled on this branch for now
  (see the TODO in input_sdl.c) - keyboard and mouse are unaffected.
