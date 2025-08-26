import os

env = SConscript("godot-cpp/SConstruct", arguments=ARGUMENTS)

env['platform'] = 'windows'
env['target'] = 'release'


env.Append(CPPPATH=["#espeak-ng-src/src/include"])
env.Append(LIBPATH=["#bin/windows"])

# Add the espeak-ng library.
env.Append(LIBS=["libespeak-ng"])

# Platform-specific LINKFLAGS (rpath for Linux, no rpath for Android/Windows)
# The godot-cpp SConstruct should handle most of the toolchain setup.
if env["platform"] == "linux":
    env.Append(LINKFLAGS=["-Wl,-rpath,'$ORIGIN'"])

# Tell SCons where our source code is.
sources = Glob("src/*.cpp")

# Determine the target output directory path and file name based on platform and architecture
target_path = f"#addons/gd-speak/bin/{env["platform"]}/libespeakgodotextension"

# Create the shared library.
library = env.SharedLibrary(
    target=target_path,
    source=sources,
)

Default(library)
