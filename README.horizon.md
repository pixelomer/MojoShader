# MojoShader for Horizon

This fork adds a headless shader context API and a standalone Horizon static-library build.
The headless implementation incorporates psyGamer's patch from
[Everest-libs](https://github.com/EverestAPI/Everest-libs/blob/3311fc9/patches/MojoShader_Headless.patch).
Upstream: [icculus/mojoshader](https://github.com/icculus/mojoshader), zlib license;
retain the upstream license and file notices. The port uses public devkitPro/libnx
homebrew interfaces. No Nintendo SDK is required.

## Build

On Linux, install Python 3.12+, Git, CMake, Ninja, make and devkitPro's switch-dev
and switch-portlibs packages. Set `DEVKITPRO` and put devkitA64/bin and tools/bin
on PATH. Build from any directory:

```sh
python3 build-horizon.py --jobs 8
```

The script fetches and builds pinned [libnx](https://github.com/pixelomer/libnx)
in ignored `artifacts/sources/`; it does not install over the system SDK.
`--libnx /path/to/sdk` optionally reuses a built SDK. Build outputs are under `artifacts/horizon/`. Keep symbols for application debugging.
Full commit pins are in `eng/horizon/dependencies.json` and Git submodule entries.
`--source-mirrors FILE.json` can map canonical URLs to local Git source mirrors;
mirrors supply source objects, never prebuilt libraries.

Archive output: `libmojoshader.a`.
Use the paired [FNA](https://github.com/pixelomer/FNA) and
[FNA3D](https://github.com/pixelomer/FNA3D) revisions selected by the application.
These builds support OpenGL integration and do not enable a Vulkan renderer.

The source-build helpers' recursive source-fetch controls can be run with
`python3 tests/horizon/test_sources.py`; these tests create only temporary,
original Git fixtures and do not require a console or game files.
