#!/usr/bin/python2.4
# Copyright 2009 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Build tool setup for MacOS.

This module is a SCons tool which should be include in the topmost mac
environment.
It is used as follows:
  env = base_env.Clone(tools = ['component_setup'])
  mac_env = base_env.Clone(tools = ['target_platform_mac'])
"""


import SCons.Script


def ComponentPlatformSetup(env, builder_name):
  """Hook to allow platform to modify environment inside a component builder.

  Args:
    env: Environment to modify
    builder_name: Name of the builder
  """
  if env.get('ENABLE_EXCEPTIONS'):
    env.FilterOut(CCFLAGS=['-fno-exceptions'])
    env.Append(CCFLAGS=['-fexceptions'])

#------------------------------------------------------------------------------

def generate(env):
  # NOTE: SCons requires the use of this name, which fails gpylint.
  """SCons entry point for this tool."""

  # Preserve some variables that get blown away by the tools.
  saved = dict()
  for k in ['ASFLAGS', 'CFLAGS', 'CCFLAGS', 'CXXFLAGS', 'LINKFLAGS', 'LIBS']:
    saved[k] = env.get(k, [])
    env[k] = []

  # Use g++
  env.Tool('g++')
  env.Tool('gcc')
  env.Tool('gnulink')
  env.Tool('ar')
  env.Tool('as')
  env.Tool('applelink')

  # Set target platform bits
  env.SetBits('mac', 'posix')

  env.Replace(
      TARGET_PLATFORM='MAC',
      COMPONENT_PLATFORM_SETUP=ComponentPlatformSetup,

      # Code coverage related.
      COVERAGE_CCFLAGS=['--coverage', '-DCOVERAGE'],
      COVERAGE_LIBS=['profile_rt'],
      COVERAGE_LINKFLAGS=['--coverage'],
      COVERAGE_STOP_CMD=[
          '$COVERAGE_MCOV --directory "$TARGET_ROOT" --output "$TARGET"',
          ('$COVERAGE_GENHTML --output-directory $COVERAGE_HTML_DIR '
           '$COVERAGE_OUTPUT_FILE'),
      ],

      # Libraries expect to be in the same directory as their executables.
      # This is correct for unit tests, and for libraries which are published
      # in Contents/MacOS next to their executables.
      DYLIB_INSTALL_NAME_FLAGS=[
          '-install_name',
          '@loader_path/${TARGET.file}'
      ],
  )

  env.Append(
      # Mac apps and dylibs have a more strict relationship about where they
      # expect to find each other.  When an app is linked, it stores the
      # relative path from itself to any dylibs it links against.  Override
      # this so that it will store the relative path from $LIB_DIR instead.
      # This is similar to RPATH on Linux.
      LINKFLAGS = [
          '-Xlinker', '-executable_path',
          '-Xlinker', '$LIB_DIR',
      ],
      # Similarly, tell the library where it expects to find itself once it's
      # installed.
      SHLINKFLAGS = ['$DYLIB_INSTALL_NAME_FLAGS'],

      # Settings for debug
      CCFLAGS_DEBUG=[
          '-O0',     # turn off optimizations
          '-g',      # turn on debugging info
      ],
      LINKFLAGS_DEBUG=['-g'],

      # Settings for optimized
      # Optimized for space by default, which is what Xcode does
      CCFLAGS_OPTIMIZED=['-Os'],

      # Settings for component_builders
      COMPONENT_LIBRARY_LINK_SUFFIXES=['.dylib', '.a'],
      COMPONENT_LIBRARY_DEBUG_SUFFIXES=[],
  )

  # Restore saved flags.
  env.Append(**saved)
