Fork of https://chromium.googlesource.com/native_client/src/native_client
with edits to enable building NaCl without the Chromium
dev tools. History had to be cut off due to 200 MB zip files in the history.
Currently it only works for amd64 host+target. (run.py won't work
since NaCl targets aren't supported yet.)
Dependencies:
- LLVM (must be installed in /usr/bin)
- SCons

### Build the NaCl loader and boostrap loader
```
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
```
scons --mode=nacl saigo=1 platform=x86-64 irt_core_raw [optional saigo_newlib_dir=...]
```

### Try some tests
This builds both components and runs some tests.
```
scons --mode=opt-host,nacl saigo=1 platform=x86-64 --keep-going small_tests medium_tests

To enable crash dump tests, add the option `breakpad_tools_dir=<path to breakpad>`. The
repository can be found at `daemon/libs/breakpad`. You need to have built the Breakpad
tools in-source.
```
---

#  Native Client

Native Client is a deprecated sandboxing technology. It is currently used by a
few third party chrome extensions and chrome apps. Many of the original project
pages are not longer available. Some documentation can be found at:
 * Documentation for [contributors to Native
   Client](http://www.chromium.org/nativeclient)
 * [Research
   papers](http://www.chromium.org/nativeclient/reference/research-papers)

##  Directory structure

The following list describes major files and directories that you'll see in
your working copy of the repository, including some directories that don't
exist until you've built Native Client. Paths are relative to the
`native_client` directory.

 * `COPYING NOTICE README.md RELEASE_NOTES documentation/`: Documentation,
   release, and license information.

 * `SConstruct scons.bat scons scons-out/ site_scons/`: Build-related files.
   The `scons.bat` and `scons` files, with data from `SConstruct`, let you
   build Native Client and its tests. The `scons-out` and `site-scons`
   directories don't exist in the git repository; they're created when Native
   Client is built. The `scons-out/*/staging` directories contain files, such
   as the Native Client plug-in and compiled examples, that let you use and
   test Native Client.

 * `src/`: Core source code for Native Client.

 * `src/include/`: Header files that are missing from some platforms and are
   used by more than one major part of Native Client

 * `src/shared/`: Source code that's used by both trusted code (such as the
   service runtime) and untrusted code (such as Native Client modules)

 * `src/third_party`: Other people's source code

 * `src/trusted/`: Source code that's used only by trusted code

 * `src/untrusted/`: Source code that's used only by untrusted code

 * `tests/common/`: Source code for examples and tests.

 * `../third_party/`: Third-party source code and binaries that aren't part of
   the service runtime.  When built, the Native Client toolchain is in
   `src/third_party/nacl_sdk/`.

 * `tools/`: Utilities such as the plug-in installer.
