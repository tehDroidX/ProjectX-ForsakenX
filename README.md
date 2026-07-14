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

## Branch `gl3.2-sdl2.30` - GL 3.2 core + SDL 2.30.9 (incl. the historic SDL2 sound fix)

SDL2 port of the GL3 backend, plus the input and sound fixes that make an
SDL2 build of this game actually playable for the first time.

### Build
`build\preview\compile-client.bat` then `link-client.bat` produce
`projectx_client_gl3.exe`. Ship `SDL2.dll` (in `build/preview/deps/`) next
to the exe; `deps/OpenAL32.dll` is openal-soft 1.25.2 (recommended).

### Fixes in this branch (on top of gl2-sdl1.2)
1. **Real GL context under SDL2**: the old SDL2 path used
   `SDL_CreateRenderer`, which never makes a GL context current - every raw
   GL call the game issues hit a dead context (`glGetString` returned NULL).
   Now: `SDL_GL_CreateContext` + `SDL_GL_MakeCurrent` + `SDL_GL_SwapWindow`,
   GL 3.2 core profile, 24-bit depth buffer explicitly requested.
2. **glBlendColor**: SDL2's SDL_opengl.h prototypes it, so the loader defines
   a forwarding function instead of a pointer. `glGetStringi` added to the
   loader, and the loader now runs at the top of `render_init()` because
   print_info/detect_caps need glGetStringi under core profiles.
3. **Working in-game video settings** (menu + Shift+F12): picking a concrete
   resolution switches to an exclusive fullscreen mode of that size
   (`SDL_SetWindowDisplayMode` + `SDL_WINDOW_FULLSCREEN`) or resizes and
   recenters the window in windowed mode; the "default" entry keeps the
   borderless desktop fullscreen. The GL context survives every variant, and
   all window-geometry state the game reads (mode index, aspect ratio, HUD
   scale, 2D y-flip, saved config) is kept in sync with the real drawable
   size.
4. **Keyboard bindings normalised to scancodes**: bindings index a 512-entry
   key-state array fed by `SDL_GetKeyboardState` (scancodes), but the config
   defaults were SDL keycodes. SDL2 keycodes for arrows etc. are 0x4000xxxx -
   they cannot index that array (and were even misclassified as joystick
   codes), which is why ship navigation was dead on SDL2 builds. Defaults,
   key-name resolution (`SDL_GetScancodeName`) and the rebinding menu
   (keycode -> `SDL_GetScancodeFromKey`) all live in scancode space now.
   Config files store key *names* and are matched case-insensitively, so
   existing configs keep working.
5. **Relative mouse mode**: without it the hidden cursor stops at the window
   border and mouse deltas die with it (the "mouse hits invisible walls"
   bug).
6. **The SDL2 sound fix**: `sound_load` passed `&wav_spec.size` as
   SDL_LoadWAV's length out-parameter - aliasing a field of the very spec
   SDL fills. SDL 1.2's internal write order let the value survive; SDL2
   clobbers it, so OpenAL received garbage-sized buffers and played silence
   while reporting AL_PLAYING with no error. A separate `wav_len` variable
   fixes it. This is most likely why past SDL2 builds of the port "had no
   sound".
