# Native Client loader

This project makes possible to rebuild the Native Client loader for usage with the [Dæmon game engine](https://github.com/DaemonEngine/Daemon). The Dæmon engine is the open-source game engine powering the [Unvanquished game](https://unvanquished.net). The Dæmon engine uses Native Client to securely and portably run downloadable compiled games.

Native Client is a sandboxing technology by Google, it was used by Chrome extensions and Chrome apps.

Google publicly annouced [in May of 2017](https://www.tomshardware.com/news/chrome-deprecates-pnacl-embraces-webassembly%2C34583.html) the (then-)upcoming deprecation and abandonment of Native Client technologies in favor of WebAssembly, and announced the actual deprecation [in 2020](https://developer.chrome.com/deprecated). But Google also [supported](https://developer.chrome.com/docs/native-client) the Native Client-powered ChromeOS 138 [until July of 2025](https://support.google.com/chrome/a/answer/10314655) and as such continued developpement of some Native Client technologies.

The related project to rebuild the Saigo Native Client compiler can be found there:

- [github.com/DaemonEngine/saigo-release-scripts](https://github.com/DaemonEngine/saigo-release-scripts)

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

Currently it only works for amd64 host+target (`run.py` won't work
since NaCl targets aren't supported yet).


## Dependencies

- LLVM (must be installed in `/usr/bin`)
- SCons


## Build the NaCl loader and boostrap loader

```sh
scons --mode=opt-host platform=x86-64 sel_ldr
```


### Build the IRT

This requires the Saigo NaCl toolchain. You can provide it by either
(a) passing `saigo_newlib_dir=<path>` on the command line (the directory
you want to target is normally called `saigo_newlib`), or
(b) dropping the toolchain in `toolchain/linux_x86/` and renaming its
top-level directory from `saigo_newlib` to `saigo_newlib_raw`.

The following command builds one `irt_core_raw.nexe`. You need to strip it
yourself; ordinary Linux `strip` seems to work.

```sh
scons --mode=nacl saigo=1 platform=x86-64 irt_core_raw [optional saigo_newlib_dir=...]
```


## Try some tests

This builds both components and runs some tests.

```sh
scons --mode=opt-host,nacl saigo=1 platform=x86-64 --keep-going small_tests medium_tests
```

To enable crash dump tests, add the option `breakpad_install_dir=<breakpad install prefix>`,
OR install Breakpad to toolchain/linux_x86/breakpad/`. The
repository can be found at `daemon/libs/breakpad`. You need to build the Breakpad
tools and run `make install`.
```
---


## Directory structure

The following list describes major files and directories that you'll see in
your working copy of the repository, including some directories that don't
exist until you've built Native Client. Paths are relative to the
`native_client` directory.

- `COPYING NOTICE README.md RELEASE_NOTES documentation/`: Documentation,
  release, and license information.
- `SConstruct scons.bat scons scons-out/ site_scons/`: Build-related files.
  The `scons.bat` and `scons` files, with data from `SConstruct`, let you
  build Native Client and its tests. The `scons-out` and `site-scons`
  directories don't exist in the git repository; they're created when Native
  Client is built. The `scons-out/*/staging` directories contain files, such
  as the Native Client plug-in and compiled examples, that let you use and
  test Native Client.
- `src/`: Core source code for Native Client.
- `src/include/`: Header files that are missing from some platforms and are
  used by more than one major part of Native Client
- `src/shared/`: Source code that's used by both trusted code (such as the
  service runtime) and untrusted code (such as Native Client modules)
- `src/third_party`: Other people's source code
- `src/trusted/`: Source code that's used only by trusted code
- `src/untrusted/`: Source code that's used only by untrusted code
- `tests/common/`: Source code for examples and tests.
- `../third_party/`: Third-party source code and binaries that aren't part of
  the service runtime.  When built, the Native Client toolchain is in
  `src/third_party/nacl_sdk/`.
- `tools/`: Utilities such as the plug-in installer (deprecated).
