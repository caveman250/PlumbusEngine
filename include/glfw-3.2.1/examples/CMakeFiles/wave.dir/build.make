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
include include/glfw-3.2.1/examples/CMakeFiles/wave.dir/depend.make

# Include the progress variables for this target.
include include/glfw-3.2.1/examples/CMakeFiles/wave.dir/progress.make

# Include the compile flags for this target's objects.
include include/glfw-3.2.1/examples/CMakeFiles/wave.dir/flags.make

include/glfw-3.2.1/examples/CMakeFiles/wave.dir/wave.c.o: include/glfw-3.2.1/examples/CMakeFiles/wave.dir/flags.make
include/glfw-3.2.1/examples/CMakeFiles/wave.dir/wave.c.o: include/glfw-3.2.1/examples/wave.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jason/Documents/VulkanRenderer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object include/glfw-3.2.1/examples/CMakeFiles/wave.dir/wave.c.o"
	cd /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/examples && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/wave.dir/wave.c.o   -c /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/examples/wave.c

include/glfw-3.2.1/examples/CMakeFiles/wave.dir/wave.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/wave.dir/wave.c.i"
	cd /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/examples && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/examples/wave.c > CMakeFiles/wave.dir/wave.c.i

include/glfw-3.2.1/examples/CMakeFiles/wave.dir/wave.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/wave.dir/wave.c.s"
	cd /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/examples && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/examples/wave.c -o CMakeFiles/wave.dir/wave.c.s

include/glfw-3.2.1/examples/CMakeFiles/wave.dir/wave.c.o.requires:

.PHONY : include/glfw-3.2.1/examples/CMakeFiles/wave.dir/wave.c.o.requires

include/glfw-3.2.1/examples/CMakeFiles/wave.dir/wave.c.o.provides: include/glfw-3.2.1/examples/CMakeFiles/wave.dir/wave.c.o.requires
	$(MAKE) -f include/glfw-3.2.1/examples/CMakeFiles/wave.dir/build.make include/glfw-3.2.1/examples/CMakeFiles/wave.dir/wave.c.o.provides.build
.PHONY : include/glfw-3.2.1/examples/CMakeFiles/wave.dir/wave.c.o.provides

include/glfw-3.2.1/examples/CMakeFiles/wave.dir/wave.c.o.provides.build: include/glfw-3.2.1/examples/CMakeFiles/wave.dir/wave.c.o


include/glfw-3.2.1/examples/CMakeFiles/wave.dir/__/deps/glad.c.o: include/glfw-3.2.1/examples/CMakeFiles/wave.dir/flags.make
include/glfw-3.2.1/examples/CMakeFiles/wave.dir/__/deps/glad.c.o: include/glfw-3.2.1/deps/glad.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jason/Documents/VulkanRenderer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object include/glfw-3.2.1/examples/CMakeFiles/wave.dir/__/deps/glad.c.o"
	cd /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/examples && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/wave.dir/__/deps/glad.c.o   -c /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/deps/glad.c

include/glfw-3.2.1/examples/CMakeFiles/wave.dir/__/deps/glad.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/wave.dir/__/deps/glad.c.i"
	cd /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/examples && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/deps/glad.c > CMakeFiles/wave.dir/__/deps/glad.c.i

include/glfw-3.2.1/examples/CMakeFiles/wave.dir/__/deps/glad.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/wave.dir/__/deps/glad.c.s"
	cd /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/examples && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/deps/glad.c -o CMakeFiles/wave.dir/__/deps/glad.c.s

include/glfw-3.2.1/examples/CMakeFiles/wave.dir/__/deps/glad.c.o.requires:

.PHONY : include/glfw-3.2.1/examples/CMakeFiles/wave.dir/__/deps/glad.c.o.requires

include/glfw-3.2.1/examples/CMakeFiles/wave.dir/__/deps/glad.c.o.provides: include/glfw-3.2.1/examples/CMakeFiles/wave.dir/__/deps/glad.c.o.requires
	$(MAKE) -f include/glfw-3.2.1/examples/CMakeFiles/wave.dir/build.make include/glfw-3.2.1/examples/CMakeFiles/wave.dir/__/deps/glad.c.o.provides.build
.PHONY : include/glfw-3.2.1/examples/CMakeFiles/wave.dir/__/deps/glad.c.o.provides

include/glfw-3.2.1/examples/CMakeFiles/wave.dir/__/deps/glad.c.o.provides.build: include/glfw-3.2.1/examples/CMakeFiles/wave.dir/__/deps/glad.c.o


# Object files for target wave
wave_OBJECTS = \
"CMakeFiles/wave.dir/wave.c.o" \
"CMakeFiles/wave.dir/__/deps/glad.c.o"

# External object files for target wave
wave_EXTERNAL_OBJECTS =

include/glfw-3.2.1/examples/wave: include/glfw-3.2.1/examples/CMakeFiles/wave.dir/wave.c.o
include/glfw-3.2.1/examples/wave: include/glfw-3.2.1/examples/CMakeFiles/wave.dir/__/deps/glad.c.o
include/glfw-3.2.1/examples/wave: include/glfw-3.2.1/examples/CMakeFiles/wave.dir/build.make
include/glfw-3.2.1/examples/wave: include/glfw-3.2.1/src/libglfw3.a
include/glfw-3.2.1/examples/wave: /usr/lib/x86_64-linux-gnu/librt.so
include/glfw-3.2.1/examples/wave: /usr/lib/x86_64-linux-gnu/libm.so
include/glfw-3.2.1/examples/wave: /usr/lib/x86_64-linux-gnu/libX11.so
include/glfw-3.2.1/examples/wave: /usr/lib/x86_64-linux-gnu/libXrandr.so
include/glfw-3.2.1/examples/wave: /usr/lib/x86_64-linux-gnu/libXinerama.so
include/glfw-3.2.1/examples/wave: /usr/lib/x86_64-linux-gnu/libXxf86vm.so
include/glfw-3.2.1/examples/wave: /usr/lib/x86_64-linux-gnu/libXcursor.so
include/glfw-3.2.1/examples/wave: include/glfw-3.2.1/examples/CMakeFiles/wave.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/jason/Documents/VulkanRenderer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable wave"
	cd /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/wave.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
include/glfw-3.2.1/examples/CMakeFiles/wave.dir/build: include/glfw-3.2.1/examples/wave

.PHONY : include/glfw-3.2.1/examples/CMakeFiles/wave.dir/build

include/glfw-3.2.1/examples/CMakeFiles/wave.dir/requires: include/glfw-3.2.1/examples/CMakeFiles/wave.dir/wave.c.o.requires
include/glfw-3.2.1/examples/CMakeFiles/wave.dir/requires: include/glfw-3.2.1/examples/CMakeFiles/wave.dir/__/deps/glad.c.o.requires

.PHONY : include/glfw-3.2.1/examples/CMakeFiles/wave.dir/requires

include/glfw-3.2.1/examples/CMakeFiles/wave.dir/clean:
	cd /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/examples && $(CMAKE_COMMAND) -P CMakeFiles/wave.dir/cmake_clean.cmake
.PHONY : include/glfw-3.2.1/examples/CMakeFiles/wave.dir/clean

include/glfw-3.2.1/examples/CMakeFiles/wave.dir/depend:
	cd /home/jason/Documents/VulkanRenderer && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jason/Documents/VulkanRenderer /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/examples /home/jason/Documents/VulkanRenderer /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/examples /home/jason/Documents/VulkanRenderer/include/glfw-3.2.1/examples/CMakeFiles/wave.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : include/glfw-3.2.1/examples/CMakeFiles/wave.dir/depend

