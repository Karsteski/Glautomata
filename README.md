# Glautomata - Conway's Game of Life

John Conway's Game of Life displayed using batch rendering in OpenGL

<img src="resources/glautomata-video.gif" alt="Glautomata-GIF" height="500">

## Get Project

```bash
git clone --recursive https://github.com/Karsteski/Glautomata.git
```

## Building Project

Project is self-contained and has been built on Linux.
It doesn't currently build as a self-contained project on Windows because of an [issue with how Meson invokes the Microsoft Resource Compiler](https://github.com/mesonbuild/meson/issues/4105) for the GLEW library.
GLEW can be in installed and linked separately into the project for Windows, using [vcpkg](https://vcpkg.io/en/index.html) or your package manager of choice.

```bash
# User must have the Meson build system installed.
# Project must be built with C++17 or later.

## Linux instructions

$ meson setup builddir
$ cd builddir
$ meson configure -Dcpp_std=c++17
$ meson configure -Dbuildtype=release
$ meson compile
$ ./glautomata
```

## Usage

- The Game of Life automatically runs once executable is started.
- Press *spacebar* to regenerate the game once it's run its course.
- Enjoy :)

## License

MIT License

Copyright (c) 2021 Kareem Stephan Skinner

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
