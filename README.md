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

## Branch `gl4.6-dsa-sdl3.4` - native GL 4.6 renderer (DSA + GLSL 460)

Builds `projectx_client_gl4x.exe` (`-DGL=4`, new `render_gl4.c`): the same
architecture as the GL3 backend (VAO cache + shadow buffers) rebuilt on
modern GL:

* **Direct State Access everywhere** - buffers (`glCreateBuffers`,
  `glNamedBufferData/SubData`), vertex arrays (`glCreateVertexArrays` +
  `glVertexArrayVertexBuffer/AttribFormat/AttribBinding/ElementBuffer`) and
  textures (`glCreateTextures`, `glTextureStorage2D` immutable with a full
  mip chain, `glTextureSubImage2D`, `glGenerateTextureMipmap`,
  `glBindTextureUnit`) are created and updated without touching bind points.
* **GLSL 460 with explicit `layout(location)`** for every attribute and
  uniform (see the `FSK_*` constants in render_gl_shared.h) - zero
  `glGet*Location` calls in the whole renderer.
* anisotropic filtering read as a GL 4.6 core property.
* **KHR_debug output** (opt-in: `FSKGLDEBUG=1`) - driver messages go to the
  game log without any glGetError polling.
* deliberately **no persistent-mapped buffers**: they hand back the same
  write-combined memory class whose read-modify-write cost caused the
  original framerate collapse; shadow copy + upload is the right shape for
  this engine's per-frame vertex work.

Performance is on par with the GL3 branch (the engine is CPU-bound) - this
branch exists to prove the codebase runs a clean, fully modern GL path.
