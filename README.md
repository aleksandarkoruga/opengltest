# OpenGLTest

Author: Aleksandar Koruga

OpenGL render engine for supercollider.
Check either c++ code or sc files for details, if you're looking for a compiled version, check https://github.com/aleksandarkoruga/public-extensions/tree/main/OpenGLTest

TODO: 
- uniform values control from sc - dynamically assign uniforms from input control rate array
- specify vertex array from buffer
- camera control - position, lookat
  
- Buffer variations of plugin ->              Audio in - Buffer out | Buffer In - Buffer Out | Buffer In - Audio Out
- Maybe add visualizer only possibility?      Audio In - No out | Buffer In - No Out
- compute shaders


### Requirements

- CMake >= 3.5
- SuperCollider source code
- GLM
- GLFW
- GLUT
- GLEW

### Building

Clone the project:

    git clone https://github.com/aleksandarkoruga/opengltest
    cd opengltest
    mkdir build
    cd build

Get GLM, GLFW, GLUT, GLEW libraries 

update CMakeLists include directories:
    PATH_TO_GLM
    PATH_TO_GLFW
    PATH_TO_GLEW
    PATH_TO_GLUT

Then, use CMake to configure and build it:

    cmake .. -DCMAKE_BUILD_TYPE=Release
    cmake --build . --config Release
    cmake --build . --config Release --target install

You may want to manually specify the install location in the first step to point it at your
SuperCollider extensions directory: add the option `-DCMAKE_INSTALL_PREFIX=/path/to/extensions`.

It's expected that the SuperCollider repo is cloned at `../supercollider` relative to this repo. If
it's not: add the option `-DSC_PATH=/path/to/sc/source`.

### Developing

Use the command in `regenerate` to update CMakeLists.txt when you add or remove files from the
project. You don't need to run it if you only change the contents of existing files. You may need to
edit the command if you add, remove, or rename plugins, to match the new plugin paths. Run the
script with `--help` to see all available options.
