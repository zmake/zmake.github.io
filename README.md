# zmake

zmake is a build system generator like CMake or SCons, but designed to be light-weight. Its opt-out file enumeration enables a compilation unit to be added by simply creating the file. It also enables filenames and directories to define which configurations they should be compiled for.

## What does it do?

zmake is intended to run as a pre-build step for make or Visual Studio. It recurses from specified root directories and overwrites out of date vcxproj and Makefiles.

Examples of files zmake generates: [Makefile](../master/src/zmake/Makefile), [vcxproj](../master/.zproj/.%20zmake.vcxproj)


## What does it not do?

zmake does not manage compiler options. Instead generated vcxprojs import zmake_base.props and either zmake_exe.props, zmake_lib.props, or zmake_dll.props. Generated Makefiles include zmake_top.mk and zmake_base.mk. Users choose their platform specific compiler options.

Currently it does not support Xcode (because of the pbxproj file format). If you have documentation on this format, I can add support.

## How do I configure it?

zmake-config.ztxt is the global settings file. The complete list of parameters is defined in [src/zmake/Config.cpp](../master/src/zmake/Config.cpp) (look for WTON_MEMBER)

Individual directories may include parameter overrides with a file named zmake.ztxt. The complete list of overrideable parameters is defined in [src/zmake/ProjectData.cpp](../master/src/zmake/ProjectData.cpp) (look for WTON_MEMBER)

zmake is built with zmake. Look at its examples directory for many use cases.
