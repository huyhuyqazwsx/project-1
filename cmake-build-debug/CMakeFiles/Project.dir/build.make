# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.30

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\JetBrains\CLion 2024.3\bin\cmake\win\x64\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\JetBrains\CLion 2024.3\bin\cmake\win\x64\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\HUY\CLionProjects\testlibzip

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\HUY\CLionProjects\testlibzip\cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/Project.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/Project.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/Project.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Project.dir/flags.make

CMakeFiles/Project.dir/main.cpp.obj: CMakeFiles/Project.dir/flags.make
CMakeFiles/Project.dir/main.cpp.obj: CMakeFiles/Project.dir/includes_CXX.rsp
CMakeFiles/Project.dir/main.cpp.obj: C:/Users/HUY/CLionProjects/testlibzip/main.cpp
CMakeFiles/Project.dir/main.cpp.obj: CMakeFiles/Project.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\Users\HUY\CLionProjects\testlibzip\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/Project.dir/main.cpp.obj"
	C:\mingw64\bin\c++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/Project.dir/main.cpp.obj -MF CMakeFiles\Project.dir\main.cpp.obj.d -o CMakeFiles\Project.dir\main.cpp.obj -c C:\Users\HUY\CLionProjects\testlibzip\main.cpp

CMakeFiles/Project.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/Project.dir/main.cpp.i"
	C:\mingw64\bin\c++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\HUY\CLionProjects\testlibzip\main.cpp > CMakeFiles\Project.dir\main.cpp.i

CMakeFiles/Project.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/Project.dir/main.cpp.s"
	C:\mingw64\bin\c++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\HUY\CLionProjects\testlibzip\main.cpp -o CMakeFiles\Project.dir\main.cpp.s

# Object files for target Project
Project_OBJECTS = \
"CMakeFiles/Project.dir/main.cpp.obj"

# External object files for target Project
Project_EXTERNAL_OBJECTS =

Project.exe: CMakeFiles/Project.dir/main.cpp.obj
Project.exe: CMakeFiles/Project.dir/build.make
Project.exe: C:/Users/HUY/.vcpkg-clion/vcpkg/installed/x64-mingw-dynamic/debug/lib/libzlibd.dll.a
Project.exe: C:/Users/HUY/.vcpkg-clion/vcpkg/installed/x64-mingw-dynamic/debug/lib/libzip.dll.a
Project.exe: CMakeFiles/Project.dir/linkLibs.rsp
Project.exe: CMakeFiles/Project.dir/objects1.rsp
Project.exe: CMakeFiles/Project.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=C:\Users\HUY\CLionProjects\testlibzip\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable Project.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\Project.dir\link.txt --verbose=$(VERBOSE)
	C:\Windows\System32\WindowsPowerShell\v1.0\powershell.exe -noprofile -executionpolicy Bypass -file C:/Users/HUY/.vcpkg-clion/vcpkg/scripts/buildsystems/msbuild/applocal.ps1 -targetBinary C:/Users/HUY/CLionProjects/testlibzip/cmake-build-debug/Project.exe -installedDir C:/Users/HUY/.vcpkg-clion/vcpkg/installed/x64-mingw-dynamic/debug/bin -OutVariable out

# Rule to build all files generated by this target.
CMakeFiles/Project.dir/build: Project.exe
.PHONY : CMakeFiles/Project.dir/build

CMakeFiles/Project.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\Project.dir\cmake_clean.cmake
.PHONY : CMakeFiles/Project.dir/clean

CMakeFiles/Project.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\HUY\CLionProjects\testlibzip C:\Users\HUY\CLionProjects\testlibzip C:\Users\HUY\CLionProjects\testlibzip\cmake-build-debug C:\Users\HUY\CLionProjects\testlibzip\cmake-build-debug C:\Users\HUY\CLionProjects\testlibzip\cmake-build-debug\CMakeFiles\Project.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/Project.dir/depend

