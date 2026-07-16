# Native Client

This project makes possible to rebuild Native Client for usage with the
[Dæmon game engine](https://github.com/DaemonEngine/Daemon). Chromium dev tools are NOT required
to build. The Dæmon engine is the open-source game engine powering the
[Unvanquished game](https://unvanquished.net). The Dæmon engine uses Native Client to securely
and portably run downloadable compiled games.

Native Client is a sandboxing technology by Google. It was used by Chrome extensions and Chrome apps.
Google publicly annouced [in May of 2017](https://www.tomshardware.com/news/chrome-deprecates-pnacl-embraces-webassembly%2C34583.html)
the (then-)upcoming deprecation and abandonment of Native Client technologies in favor of WebAssembly,
and announced the actual deprecation [in 2020](https://developer.chrome.com/deprecated). But Google
also [supported](https://developer.chrome.com/docs/native-client) the Native Client-powered ChromeOS
138 [until July of 2025](https://support.google.com/chrome/a/answer/10314655) and as such continued
development of some Native Client technologies.

The related project to rebuild the Saigo Native Client compiler can be found there:
- [github.com/DaemonEngine/saigo-nacl-sdk](https://github.com/DaemonEngine/saigo-nacl-sdk)

Nothing about Native Client should be expected from Google anymore.


## History

This is a fork of the upstream repository:

- [chromium.googlesource.com/native_client/src/native_client](https://chromium.googlesource.com/native_client/src/native_client)

The Git history has been rewritten to remove very large files (multiple MinGW archives were stored in Git and things like that!), reducing the history size from 384MB to 57MB.

More information about this history rewriting can be found there:

- [github.com/DaemonEngine/native_client/issues/13](https://github.com/DaemonEngine/native_client/issues/13)

This fork brings edits to enable the building of NaCl without the Chromium
dev tools.

Many of the original project pages are no longer available. Some documentation about Native Client can be found at:

- Documentation for [contributors to Native Client](https://web.archive.org/web/20250323050839/https://www.chromium.org/nativeclient/) (Web archive)
- [Research papers](https://web.archive.org/web/20250821150630/https://www.chromium.org/nativeclient/reference/research-papers/) (Web archive)


## Status

Currently the Linux amd64, Windows amd64 and Linux armhf platforms are well-tested.
However there are issues with platform qualification for ARM.
For Windows i686 and Linux i686 it builds at least.
`run.py` can be used but you must build a loader first yourself (auto-build doesn't work).

## Dependencies
- SCons
- [Saigo NaCl SDK](https://github.com/DaemonEngine/saigo-nacl-sdk)
- For Linux: LLVM, if using `--clang` (default). Must be installed in `/usr/bin`.
- For Linux: GCC, if using `--no-clang`
- For Linux: GNU Binutils
- For Windows: Visual Studio
- For Windows: MinGW

## SCons usage

The build system uses SCons, which may be invoked as `scons` and/or `python -m SCons` depending
how it is installed. Inter-architecture cross-compilation is supported, but inter-operating system
is not. Build artifacts are placed in `scons-out/` by default. The `scons-out/*/staging`
directories contain the "final" outputs of the build. Here are some commonly
used command line arguments for our build:

- `werror=0` to disable compiler warnings as errors
- `--mode` which takes a comma-separated list of toolchains to enable in the build. Some values are:
  - `opt-host`: trusted code toolchain using optimization flags for the current OS
  - `dbg-host`: trusted code toolchain with debugging flags for th ecurrent OS
  - `nacl`: NaCl toolchain to build the sandboxed code
- `saigo=1` to use the Saigo NaCl toolchain. This is the only NaCl toolchain we support so you must always add this when using the `nacl` mode.
- `--platform=x86`, `--platform=x86-64`, or `--platform=arm`: choose target architecture.
- `--verbose`: show compiler command lines and other stuff
- `-j<N>` build parallelism
- Target names, e.g. `sel_ldr` (NaCl loader), `irt_core`, `small_tests`, `medium_tests`, `all_programs`, `large_tests`, `huge_tests`, `run_<something>_test`

There is also a toolchain for host-mode tools. Its configuration is based on the same arguments
used for the trusted toolchain.

### Build the NaCl loader (and bootstrap loader if used for this platform)

```sh
scons --mode=opt-host platform=x86-64 werror=0 sel_ldr
```

On Linux, add `--no-clang` to use GCC instead of Clang.

### Build the IRT (C runtime 'dynamic library' used by NaCl code)

This requires the Saigo NaCl toolchain. You can provide it by either
(a) passing `saigo_newlib_dir=<path>` on the command line (the directory
you want to target is normally called `saigo_newlib`), or
(b) dropping the toolchain in `toolchain/linux_x86/`/`toolchain/win_x86` and renaming its
top-level directory to `saigo_newlib_raw`.

The following command builds one `irt_core.nexe`. You need to strip it
yourself; ordinary Linux `strip` seems to work.

```sh
scons --mode=nacl saigo=1 platform=x86-64 werror=0 irt_core [optional saigo_newlib_dir=...]
```


### Try some tests

This builds both components and runs some tests.

```sh
scons --mode=opt-host,nacl saigo=1 platform=x86-64 werror=0 --keep-going small_tests medium_tests -j4
```

To enable crash dump tests, add the option `breakpad_install_dir=<breakpad install prefix>`,
OR install Breakpad to toolchain/linux_x86/breakpad/`. The
repository can be found at `daemon/libs/breakpad`. You need to build the Breakpad
tools and run `make install`.
```
---

### Windows native build
Install the `scons` and `pywin32` Python modules, Visual Studio, and a MinGW toolchain for the
appropriate architecture (the latter is solely used for the assembler). Python module installation:
```
python -m pip install scons pywin32
```

You can use the traditional MSVC compiler with `--no-clang`, or `clang-cl` with `--clang`.
Specify the path to the MinGW installation with `mingw_dir=...`. For example:

```
python -m SCons --mode=nacl,opt-windows --no-clang saigo=1 werror=0 mingw_dir=C:\mingw\x86_64-msvcrt-12.2.0 platform=x86-64 sel_ldr irt_core`
```

--

## Directory structure

The following list describes major files and directories in the source tree.

- `COPYING NOTICE README.md RELEASE_NOTES documentation/`: Documentation,
  release, and license information.
- `SConstruct site_scons/ build/`: Build system files.
- `src/`: Core source code for Native Client.
- `src/include/`: Header files that are missing from some platforms and are
  used by more than one major part of Native Client
- `src/shared/`: Source code that's used by both trusted code (such as the
  service runtime) and untrusted code (such as Native Client modules)
- `src/third_party`: Other people's source code
- `src/trusted/`: Source code that's used only by trusted code
- `src/untrusted/`: Source code that's used only by untrusted code
- `tests/common/`: Source code for examples and tests.
- `tools/`: Some scripts used by the build system, plus a lot of stuff we don't use.
