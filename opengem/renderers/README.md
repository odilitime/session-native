# OpenGem :: Renderer
Renderer is a cross-platform abstraction layer around OpenGL and SDL (1&2)'s software renderer. 
Also handles window management (SDL1 only supports a single window), time, cursor and input (keyboard and mouse) and other system events.
It uses OpenGem's "datastructures" package.

# Goals
- Hardware acceleration support but not required
  - OpenGL 2.x and up support
  - Vulkan support
- Supports OTF/TTF fonts
- Supports "Sprites" (i.e. textures)
- Hardware accelerated scrolling if available
- Supports systems that can't use X windows (SDL1 provides framebuffer support)
- Support dual renderes in same runtime (New SDL window, New GLFW window)
- Minimal required dependencies
- 0,0 is upper left
- HiDPI support
- Lots of user control
- can build on Mac OS X 10.6+

# What is OpenGem

OpenGem is a toolkit for developing cross-platform native application, such as a web browsers.
Hoping for something better than electron. Low dependencies so it's easy to embed in other projects.
This is one of the composable parts, renderer.

# Build requirements

- [cmake 3.8+](https://cmake.org/)
- a c compiler such as gcc (4.2+) or llvm (mainly dev'd on llvm)
- [FreeType2](https://www.freetype.org/)
- [GLFW](https://www.glfw.org/), [SDL1](https://www.libsdl.org/download-1.2.php) and/or [SDL2](https://www.libsdl.org/download-2.0.php)
   - GLFW is OpenGL only (and will eventually support Vulkan)
   - SDL1 is software renderer (and will eventually also support OpenGL). This the only one that doesn't required a windowing system like X.
   - SDL2 is OpenGL with fallback to software render (will automatically select the best for your system)

# how to build

- `cmake .` runs the configuration and detection of all the libraries you have installed on your system
- `make -jX` where X is the number of CPU cores you want to use (if you use more than you have it'll slow it down)

# Options

By default it prefers GLFW and SDL1 support. We prefer SDL1 over SDL2 because it has linux frame buffer support.

USE GLFW and SDL2 `cmake -DPREFER_SDL2=ON .`
USE SDL1 only `cmake -DIGNORE_GLFW=ON .`
USE SDL2 only `cmake -DPREFER_SDL2=ON -DIGNORE_GLFW=ON .`

# How
[Discord](https://discord.gg/ffWabPn)
IRC: irc.rizon.net in #/g/netrunner

# Documentation
