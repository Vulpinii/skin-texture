<p align="center"><h1>Skin texture</h1></p>


This program allows to visualize the effect of subsurface scattering and the generation of procedural textures on hands.

Written in C ++ and using the OpenGL API.

## Features
- Fast sub surface scatering
- Procedural skin texture
- God rays

## Building
#### On Linux
**Prerequisite**: CMake

To build this program, download the source code using ``git clone https://github.com/Vulpinii/skin-texture`` or directly the zip archive.
Then run the `` launch.sh`` shell script.

You can do it manually by following these commands:
```shell script
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
./program
```

#### On Windows
[instructions coming soon]


## Gallery
#### YouTube Video

<a href="https://youtu.be/MlTtGBSUrVE" target="_blank"><img src="https://github.com/Vulpinii/skin-texture/blob/master/images/youtube_skin.PNG" 
alt="Skin Texture" width="100%" height="auto" border="10" /></a>

#### Preview
<p align="center"><img src="https://github.com/Vulpinii/skin-texture/blob/master/images/skin.gif" alt="Animated gif" width="100%" /></p>
