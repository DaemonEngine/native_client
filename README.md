# Native Client

[![Web](https://img.shields.io/badge/web-unvanquished.net-ffaaaa)](https://forums.unvanquished.net)
[![Forums](https://img.shields.io/badge/forums-forums.unvanquished.net-ffaaaa)](https://forums.unvanquished.net)
[![Wiki](https://img.shields.io/badge/wiki-wiki.unvanquished.net%20%E2%80%A3%20Native_Client-ffaaaa)](https://wiki.unvanquished.net/wiki/Native_Client)  
[![Rules](https://img.shields.io/badge/chat-rules-ffdd00)](https://wiki.unvanquished.net/wiki/Chat#Rules)
[![IRC](https://img.shields.io/badge/irc-%23unvanquished%2C%23unvanquished--dev-9cf.svg)](https://unvanquished.net/chat/)
[![Matrix](https://img.shields.io/badge/matrix-Unvanquished-9cf?logo=matrix)](https://matrix.to/#/!WnuetRiQZJNBTKwMrF:matrix.org?via=matrix.org)
[![Discord](https://img.shields.io/badge/discord-Unvanquished-9cf?logo=discord)](https://discord.gg/usuDT9Pyna)

This project makes possible to rebuild Native Client for usage with the
[Dæmon game engine](https://github.com/DaemonEngine/Daemon). Chromium development tools are **NOT** required to build.

Native Client (also known as NaCl) is a sandboxing technology by Google.
It was used by Chrome extensions and Chrome apps.
The Dæmon engine is the open-source game engine powering the [Unvanquished game](https://unvanquished.net),
it uses Native Client to securely and portably run downloadable compiled games.

Google publicly announced [in May of 2017](https://blog.chromium.org/2017/05/goodbye-pnacl-hello-webassembly.html) the (then-)upcoming deprecation and abandonment of Native Client technologies in favor of WebAssembly, and announced the actual deprecation [in 2020](https://developer.chrome.com/deprecated).
But Google also [supported](https://developer.chrome.com/docs/native-client) Native Client-powered ChromeOS 138 until ChromeOS 139 [in July of 2025](https://support.google.com/chrome/a/answer/10314655?&#139) and as such continued developpement of some Native Client technologies.
This extra development materialized in the maintenance of the loader and a toolchain named Saigo,
frequently rebased on the latest LLVM upstream at the time. The loader received commits from Google [until April of 2025](https://chromium.googlesource.com/native_client/src/native_client.git/+/e3fce84f253bc1e77bb239185c0fbff23dc8e3ee), and Saigo received commits from Google [until January of 2025](https://chromium.googlesource.com/native_client/nacl-llvm-project-v10/+/9c7f0369cfdd591e580c5ccfc1f00fedee58029f) and the last rebase was over Clang 21.

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


### ARM testing

There are some special build system features that facilitate testing for the
32-bit ARM Linux target.


#### Running tests with user-mode QEMU

Some unit tests can run directly on an x86-based host with the help of user-mode QEMU. You
don't need to pass any extra arguments to the build system, just install the necessary
software, which may be named something like "qemu-user". Then issue a normal testing command e.g.

```
scons --mode=opt-host,nacl saigo=1 platform=arm werror=0 --keep-going small_tests
```

This is useful but there are many tests that it cannot run, for example because they involve
spawning multiple process. Also it is important to test on a full machine to exercise the
arcane kernel interactions such as `longjmp`ing out of a signal handler.

#### Running tests on a different machine than they were built
To test on an emulated or slow ARM machine, you can build the binaries on another system first
and then copy everything to the test machine. You need to copy both the build system files and
the compiled artifacts. Then on the test machine, you only need to install SCons to run tests,
using the `built_elsewhere=1` flag. For example,

```
# on build machine
scons --mode=opt-linux,nacl saigo=1 platform=arm all_programs -j8
rsync -az --info=progress2 * testuser@testhost:/home/testuser/nacl-test
```

Then on the test machine,

```
scons --mode=opt-linux,nacl saigo=1 platform=arm small_tests medium_tests large_tests
```


## Using `run.py`

Once you have built the loader and IRT, you can use `run.py` to run an nexe like a normal
program. It specifies relaxed I/O permissions so the nexe can print to stdout, etc.

Note that if you did not build the dependencies, the script will try to build them
with a PNaCl toolchain (not the Saigo that we want) and probably fail.


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
