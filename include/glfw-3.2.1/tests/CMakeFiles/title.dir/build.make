# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.9

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/jason/Documents/VulkanRenderer

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/jason/Documents/VulkanRenderer

# Include any dependencies generated for this target.
include include/glfw-3.2.1/tests/CMakeFiles/title.dir/depend.make

# Include the progress variables for this target.
include include/glfw-3.2.1/tests/CMakeFiles/title.dir/progress.make

# Include the compile flags for this target's objects.
include include/glfw-3.2.1/tests/CMakeFiles/title.dir/flags.make

include/glfw-3.2.1/tests/CMakeFiles/title.dir/title.c.o: include/glfw-3.2.1/tests/CMakeFiles/title.dir/flags.make
include/glfw-3.2.1/tests/CMakeFiles/title.dir/title.c.o: include/glfw-3.2.1/tests/title.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jason/Documents/VulkanRenderer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object include/glfw-3.2.1/tests/CMakeFiles/title.dir/title.c.o"
	cd /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/tests && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/title.dir/title.c.o   -c /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/tests/title.c

include/glfw-3.2.1/tests/CMakeFiles/title.dir/title.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/title.dir/title.c.i"
	cd /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/tests && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/tests/title.c > CMakeFiles/title.dir/title.c.i

include/glfw-3.2.1/tests/CMakeFiles/title.dir/title.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/title.dir/title.c.s"
	cd /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/tests && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/tests/title.c -o CMakeFiles/title.dir/title.c.s

include/glfw-3.2.1/tests/CMakeFiles/title.dir/title.c.o.requires:

.PHONY : include/glfw-3.2.1/tests/CMakeFiles/title.dir/title.c.o.requires

include/glfw-3.2.1/tests/CMakeFiles/title.dir/title.c.o.provides: include/glfw-3.2.1/tests/CMakeFiles/title.dir/title.c.o.requires
	$(MAKE) -f include/glfw-3.2.1/tests/CMakeFiles/title.dir/build.make include/glfw-3.2.1/tests/CMakeFiles/title.dir/title.c.o.provides.build
.PHONY : include/glfw-3.2.1/tests/CMakeFiles/title.dir/title.c.o.provides

include/glfw-3.2.1/tests/CMakeFiles/title.dir/title.c.o.provides.build: include/glfw-3.2.1/tests/CMakeFiles/title.dir/title.c.o


include/glfw-3.2.1/tests/CMakeFiles/title.dir/__/deps/glad.c.o: include/glfw-3.2.1/tests/CMakeFiles/title.dir/flags.make
include/glfw-3.2.1/tests/CMakeFiles/title.dir/__/deps/glad.c.o: include/glfw-3.2.1/deps/glad.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jason/Documents/VulkanRenderer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object include/glfw-3.2.1/tests/CMakeFiles/title.dir/__/deps/glad.c.o"
	cd /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/tests && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/title.dir/__/deps/glad.c.o   -c /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/deps/glad.c

include/glfw-3.2.1/tests/CMakeFiles/title.dir/__/deps/glad.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/title.dir/__/deps/glad.c.i"
	cd /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/tests && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/deps/glad.c > CMakeFiles/title.dir/__/deps/glad.c.i

include/glfw-3.2.1/tests/CMakeFiles/title.dir/__/deps/glad.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/title.dir/__/deps/glad.c.s"
	cd /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/tests && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/deps/glad.c -o CMakeFiles/title.dir/__/deps/glad.c.s

include/glfw-3.2.1/tests/CMakeFiles/title.dir/__/deps/glad.c.o.requires:

.PHONY : include/glfw-3.2.1/tests/CMakeFiles/title.dir/__/deps/glad.c.o.requires

include/glfw-3.2.1/tests/CMakeFiles/title.dir/__/deps/glad.c.o.provides: include/glfw-3.2.1/tests/CMakeFiles/title.dir/__/deps/glad.c.o.requires
	$(MAKE) -f include/glfw-3.2.1/tests/CMakeFiles/title.dir/build.make include/glfw-3.2.1/tests/CMakeFiles/title.dir/__/deps/glad.c.o.provides.build
.PHONY : include/glfw-3.2.1/tests/CMakeFiles/title.dir/__/deps/glad.c.o.provides

include/glfw-3.2.1/tests/CMakeFiles/title.dir/__/deps/glad.c.o.provides.build: include/glfw-3.2.1/tests/CMakeFiles/title.dir/__/deps/glad.c.o


# Object files for target title
title_OBJECTS = \
"CMakeFiles/title.dir/title.c.o" \
"CMakeFiles/title.dir/__/deps/glad.c.o"

# External object files for target title
title_EXTERNAL_OBJECTS =

include/glfw-3.2.1/tests/title: include/glfw-3.2.1/tests/CMakeFiles/title.dir/title.c.o
include/glfw-3.2.1/tests/title: include/glfw-3.2.1/tests/CMakeFiles/title.dir/__/deps/glad.c.o
include/glfw-3.2.1/tests/title: include/glfw-3.2.1/tests/CMakeFiles/title.dir/build.make
include/glfw-3.2.1/tests/title: include/glfw-3.2.1/src/libglfw3.a
include/glfw-3.2.1/tests/title: /usr/lib/x86_64-linux-gnu/librt.so
include/glfw-3.2.1/tests/title: /usr/lib/x86_64-linux-gnu/libm.so
include/glfw-3.2.1/tests/title: /usr/lib/x86_64-linux-gnu/libX11.so
include/glfw-3.2.1/tests/title: /usr/lib/x86_64-linux-gnu/libXrandr.so
include/glfw-3.2.1/tests/title: /usr/lib/x86_64-linux-gnu/libXinerama.so
include/glfw-3.2.1/tests/title: /usr/lib/x86_64-linux-gnu/libXxf86vm.so
include/glfw-3.2.1/tests/title: /usr/lib/x86_64-linux-gnu/libXcursor.so
include/glfw-3.2.1/tests/title: include/glfw-3.2.1/tests/CMakeFiles/title.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/jason/Documents/VulkanRenderer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable title"
	cd /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/title.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
include/glfw-3.2.1/tests/CMakeFiles/title.dir/build: include/glfw-3.2.1/tests/title

.PHONY : include/glfw-3.2.1/tests/CMakeFiles/title.dir/build

include/glfw-3.2.1/tests/CMakeFiles/title.dir/requires: include/glfw-3.2.1/tests/CMakeFiles/title.dir/title.c.o.requires
include/glfw-3.2.1/tests/CMakeFiles/title.dir/requires: include/glfw-3.2.1/tests/CMakeFiles/title.dir/__/deps/glad.c.o.requires

.PHONY : include/glfw-3.2.1/tests/CMakeFiles/title.dir/requires

include/glfw-3.2.1/tests/CMakeFiles/title.dir/clean:
	cd /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/tests && $(CMAKE_COMMAND) -P CMakeFiles/title.dir/cmake_clean.cmake
.PHONY : include/glfw-3.2.1/tests/CMakeFiles/title.dir/clean

include/glfw-3.2.1/tests/CMakeFiles/title.dir/depend:
	cd /home/jason/Documents/VulkanRenderer && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jason/Documents/VulkanRenderer /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/tests /home/jason/Documents/VulkanRenderer /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/tests /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/tests/CMakeFiles/title.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : include/glfw-3.2.1/tests/CMakeFiles/title.dir/depend
