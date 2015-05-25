cmake_minimum_required(VERSION 3.0.99)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY, "android-bin")
project( Stepmania_Android )


if (CMAKE_SYSTEM_NAME MATCHES "Android")
  # nop
else()
  # asplode
  message(FATAL_ERROR "\
This cmake script is specifically for building Stepmania for Android.\n\
Please run using this syntax:\n\
  cmake -DCMAKE_TOOLCHAIN_FILE=Toolchain/android.toolchain.cmake")
endif()

# Ordering is critical. If this is not before StepmaniaCore, Core doesn't get the Android stuff.
include("Toolchain/AndroidNdkModules.cmake")
android_ndk_import_module_ndk_helper()

include("../../../StepmaniaCore.cmake")

# Set current dir as an includes dir.
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

# Set a relative variable because more simple.
set(PROJECT_INTERNAL_ROOTDIR "${PROJECT_SOURCE_DIR}/../../..")

if (Stepmania_Android_BINARY_DIR MATCHES "")
  set(Stepmania_Android_BINARY_DIR "./")
else()
endif()

# The external libraries need to be included.
add_subdirectory(../../../extern ${Stepmania_Android_BINARY_DIR}/extern-android)

# The internal libraries and eventual executable to be used.
add_subdirectory(../../ ${Stepmania_Android_BINARY_DIR}/android)

find_package(ZLIB)

# Proper sound lib
find_package(OpenSLES)

# Includes all things.
message(${ANDROID_NDK}/sources/android/native_app_glue)

#include_directories(${ANDROID_NDK}/sources/android/native_app_glue)
#include_directories(${ANDROID_NDK}/sources/android/ndk_helper)
#include_directories(${ANDROID_NDK}/sources/android/cpufeatures)

#android_ndk_import_module_native_app_glue()
#android_ndk_import_module_cpufeatures()

# Specifically to fool the CLion IDE into considering the root of the SMgit as an alternate source directory
set(Stepmania_Android_SOURCE_DIR "../../../")

