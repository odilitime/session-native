![CMake](https://github.com/odilitime/session-native/workflows/CMake/badge.svg)
# session-native
native implementation of Session in c without Electron using the opengem framework (Low dependencies so it's easy to embed in other projects, support opengl as well as linux framebuffer)

## Binaries
[No offical release yet, please try the CI artifacts (mac/linux) of one of the workflow run results](https://github.com/odilitime/session-native/actions?query=workflow%3ACMake)

## how to run

`./session-native`
it will look for a `Resources/` directory in the startup directory for CA bundle and ttf font.

## Milestones

### Supports

- snode communication
- Session protocol encryption (sending)
- Session protocol decryption (recieving)
- initial UI

### Working On
- Finishing MVP UI
- non-blocking net-io
- identity management
- UI design
- open group support
- profile support
- avatar / attachment support
- mutlithreading
- encrypted database
- onion routing support / lokinet support
- closed group supprt

## Build Information

### Build requirements

- [cmake 2.8.7+](https://cmake.org/)
- git
- a c compiler such as gcc (4.2.1+) or llvm (mainly dev'd on llvm)
- [FreeType2](https://www.freetype.org/)
- [GLFW](https://www.glfw.org/), [SDL1](https://www.libsdl.org/download-1.2.php) and/or [SDL2](https://www.libsdl.org/download-2.0.php)
   - GLFW is OpenGL only (and will eventually support Vulkan)
   - SDL1 is software renderer (and will eventually also support OpenGL). This the only one that doesn't required a windowing system like X.
   - SDL2 is OpenGL with fallback to software render (will automatically select the best for your system)
- libsodium 1.0.18+
- openssl (mbed support coming soon)
- opengem (cmake will automatically download via git)

#### Package shortucts
Some of these do not include mbedtls yet, if you know the correct package name for your distro, please let us know

##### Debian (and derivates)
`sudo apt-get install libfreetype6-dev libglfw3-dev`
Also will need either libmbedtls-dev (Debian 9/Ubuntu 16LTS (xenial)) or libpolarssl-dev (Debian 8 or earlier)
Looks like on Ubuntu, only zesty and newer have 2.4+ of mbedtls

##### Arch (and derivates)
`sudo pacman -Suy glew freetype2 mbedtls`

##### Void
`sudo xbps-install -S glfw`

##### Gentoo
`sudo emerge freetype glfw`

#### Mac OS X (Brew)
`sudo brew install glfw freetype mbedtls`

#### OpenBSD
Note: OpenBSD 6.1 has freetype is 1.3.1 and mbedtls is 2.2.1 which won't work (try -CURRENT)
Be sure to use gmake too
`pkg_add glew glfw freetype mbedtls`


### how to build

- `cmake .` runs the configuration and detection of all the libraries you have installed on your system
- `make -jX` where X is the number of CPU cores you want to use (if you use more than you have it'll slow it down)

### Special build options

By default it prefers GLFW and SDL1 support. We prefer SDL1 over SDL2 because it has linux frame buffer support.

USE GLFW and SDL2 `cmake -DPREFER_SDL2=ON .`
USE SDL1 only `cmake -DIGNORE_GLFW=ON .`
USE SDL2 only `cmake -DPREFER_SDL2=ON -DIGNORE_GLFW=ON .`

# Support my work

Oxen donations:
LT2mP2DrmGD82gFnH16ty8ZtP6f33czpA6XgQdnuTVeT5bNGyy3vnaUezzKq1rEYyq3cvb2GBZ5LjCC6uqDyKnbvFki9aAX
