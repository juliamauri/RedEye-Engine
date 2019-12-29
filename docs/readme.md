# Red Eye Engine [![Build status](https://ci.appveyor.com/api/projects/status/swrp9sgx89yxl493?svg=true)](https://ci.appveyor.com/project/cumus/redeye-engine)

![Frustum Culling](https://i.gyazo.com/340473d1ddace10fb78c592ee0232359.gif)
![Dynamic AABB Tree](https://i.gyazo.com/7aecf65fdd396ddcc73ab2c1be5183b7.gif)

# Click to watch a video
[![Youtube](https://img.youtube.com/vi/kDCOSGXkqL4/0.jpg)](https://www.youtube.com/watch?v=kDCOSGXkqL4)

Red Eye is a 3D Game Engine written in C++ as an assignment for a university course.

What’s special about it? The code was written from scratch using a total of 15 C++ libraries. We started by building a geometry loader that could render the scene with simple controls for the camera. Once the base was solid we moved on to optimize the rendering process (frustum culling and quadtrees) and resource management. For the finishing touches we:
* Added shader pipeline with integrated editor so that shaders can be modified, compiled and updated instantly.
* Changed quadtree for a better space partitioning algorithm: Dynamic Bound Box Tree; the same Overwatch uses.
* Added Unity-like resource management with binary save/load for all the engines resources.

## Links:
* Repository [Github](https://github.com/juliamauri/RedEye-Engine)
* Webpage: https://juliamauri.github.io/RedEye-Engine/
* Authors: [Julià Mauri Costa](https://github.com/juliamauri) & [Rubén Sardón](https://github.com/cumus)
* Tutor: [Marc Garrigó](https://github.com/markitus18)
* University: [CITM UPC](https://www.citm.upc.edu/)
* License: [GNU General Public License v3.0](https://github.com/juliamauri/RedEye-Engine/blob/master/LICENSE)

# The Team
![us](https://i.gyazo.com/a921f5ed3b659798393a16a1e6021e66.jpg)

Julià (right): shader pipeline and the resource management.
Rubén (left): transforms, cameras and space partitioning.

We are good friends and passionate programmers and have been in a constant fight with one another to keep on track with milestones. We both had amazing new ideas for the engine every week but had to write them down for another time as we had a fixed delivery date. We split or tasks as much as possible but there where always dependencies in the way and for most of the time we spent side by side tackling all the bugs.

# Application Systems
Our application has 5 main modules that are updated each frame:
* Input
* Window
* Scene
* Editor
* Renderer

Then there are a bunch of independent modules that require no update and are used all over the place:
* File system
* Managers for: timers, cameras, primitives, frame buffer objects, resources and thumbnails.
* Importers for: models, textures and shaders.
* Other: system specs, math, internal resources, error handler, glcache (optimizes repeated opengl calls).

# Shader Pipeline
The sub-system we focused on is quite simple. It's a shader manager that uses static functions to ease their handling and to make it more fun we:
* Added a frame buffer object manager to allow for multiple rendering targets.
* Made selected gameobjects render with a thin border using the stencil buffer.
* Made a sample Gerstner Wave shader we found [here](https://developer.nvidia.com/gpugems/gpugems/part-i-natural-effects/chapter-1-effective-water-simulation-physical-models).
* Added Dynamic AABB Trees to further optimize the renderer as it was starting to get laggy. These trees calculate the bounding boxes the enclose the scene using always the combination with least surface area.

![r](https://i.gyazo.com/fd1f8c59aa2b05a0e1ccb38f164ddc28.gif)

File system uses frame’s extra milliseconds to iterate the virtual directory looking for any changes in file last modification date. Thanks to this little detail we are able to detect any changes in our assets and update the shaders as soon as they are modified in-editor or outside.

Mesh components store geometry and its associated material. The material contains the shader it's going to be rendered with and the shader contains the actual program and its uniforms. Everything except debug drawing and editor UI renders using the shader pipeline. 

## **License**
![GNU General Public License v3.0](https://i.gyazo.com/6eda7e6f16542e198c312c36dbd8c1a2.png)
