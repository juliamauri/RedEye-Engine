# Red Eye Engine [![Build status](https://ci.appveyor.com/api/projects/status/swrp9sgx89yxl493?svg=true)](https://ci.appveyor.com/project/cumus/redeye-engine)

3D Game Engine Sofware for academic purposes. Loads geometry onto a Game Object Hierarchy with its materials. Same meshes only load once. Orbital camera encloses selected geometry on focus using bounding boxes.

* Repository [Github](https://github.com/juliamauri/RedEye-Engine)
* Webpage: https://juliamauri.github.io/RedEye-Engine/
* Authors: [Julià Mauri Costa](https://github.com/juliamauri) & [Rubén Sardón](https://github.com/cumus)
* Tutor: [Marc Garrigó](https://github.com/markitus18)
* University: [CITM UPC](https://www.citm.upc.edu/)
* License: [GNU General Public License v3.0](https://github.com/juliamauri/RedEye-Engine/blob/master/LICENSE)

## User Actions
### Application Controls
* F1 to toggle showing Editor
### Camera Controls
* Left click to select a game object.
* While Right clicking, free look around
* While Right clicking, WASD movement
* While Right clicking, SPACE_BAR elevates camera
* Holding SHIFT duplicates movement speed.
* Mouse wheel zooms in and out
* Alt + Left click orbits object
* F focuses editor camera on selected Game Object enclosing its geometry.
### Scene Playback Controls
* Play: only needs a main camera in scene (created at scene start by default)
* Tick: calls next frame and pauses
* Restart/Pause/Stop...
### Supported Drag & Drop file extensions
* **.zip**: If not already in /Library, file copies to /Assets and is iterated. Resources are saved in /Library with our own format and loaded to scene.
* **.fbx**: If not already in /Library, file copies to /Assets. Then searches at origin path for other files needed and copies them too. Resources are saved in /Library with our own format and loaded to scene.
* **.png, .dds, .jpg & .tga**: If not already in /Library, imports file to /Assets. If selected Game Object has a mesh, the meshes' texture is changed.

## Other Systems
### Camera Manager & Target Aspect Ratio
Cameras are handled through the new CameraManager class. Using static functions allows for clean getting of cameras anywhere in code. Using the also new events system, the cameras get the renderer's gl_Viewport updated given any window resizes.
### Quadtree
Using the events system, scene can control where all root gameobjects are. If any gameobjects exit the Quadtree boundaries after transformation, they get stored away from the quadtree (Pop) in a list with the rest of static objects outside the quadtree. If a gameobject re-enters the quadtre boundaries, its re-added to the quadtree (Push).
### Skybox
Default skybox loads on start. Using glDepthFunc(GL_LEQUAL) instead of glDepthFunc(GL_LESS) and rendered last, the skybox always draws the furthest away. For passing the 6 different textures,GL_TEXTURE_CUBE_MAP binds on draw call.
### Console & Error Handler
Engine logs all startup and importing procedures and can be accessed through the console window. This window contains a menu to filter logs shown by category and by file. Upon having to log an error report, a Pop Up appears showing the error description and the solution opted for. Additional logs and warnings about this procedure can be seen inside the same Pop Up.

### Bounding Boxes
Game Objects have local and global bounding boxes and are updated when applying transformations. Parent Game Objects enclose their child's bounding boxes but don't reset on applying transformations to a child. This step is manual and a Button for reseting them will appear in the scene's Configuration Window. Reset is automatically applied on importing new geometry to scene.

When focusing a Game Object, the camera calculates the minimum distance from the geometry which encloses it using its global bounding box.
### Materials
When importing geometry, all materials used are saved to /Assets/Materials as a Rapidjson file. Any textures contained by them are saved to /Library/Images.
### Shaders
All geometry is rendered through shaders except bounding boxes. Just using for now 2 shaders that draw geometry with a given diffuse texture or a solid color.
### MD5 & Resource Referencing
Materials, textures and meshes use an MD5 generated strings to reference each other and to quickly check if they are duplicated when importing. How it works:
* **Component Mesh** contains referenced mesh MD5.
* **Meshes** contain referenced material MD5. Its MD5 comes from its vertex data and is saved to /Library/Meshes as _(MD5).red_.
* **Materials** contain referenced diffuse texture MD5. Its MD5 comes from the .mat file buffer and is saved to /Assets/Materials as _(name).pupil_.
* **Textures** reference nothing. Its MD5 comes from its file path in /Assets and is saved to /Library/Images as _(MD5).eye_.
### Prefabs
**Internal Prefabs**
Each mesh from an imported geometry is saved separately. When a mesh is imported, an internal prefab is created from the resulting Game Object. Serialized, it contains all references to each separate mesh as a Game Object structure. These prefabs are saves to /Library/Scenes as _(name).refab_.

**External Prefabs**
Using the Prefabs window, the user can create its own external prefabs to duplicate scene's Game Objects. These prefabs are saves to /Assets/Prefabs as _(name).refab_.

### ZIP
When droping and .fbx file, its path will vary depending on which platform de engine is running on and there must be a 
Importing .fbx files may trigger errors while checking the materials' texture paths. These paths vary depending on which platform de engine is running and there must be a conversion if needed. When importing a .zip file containing all resources allows for LibZip and PhysFS to create a virtual directory through which access is already handled. This system allows cross-platform imports through .zip files.

## User Interface
### File
* **Exit**: closes engine.
### Create
* **Cube**: creates a Game Object with a Cube Primitive component using par_shapes.
* **Sphere**: creates a Game Object with a Sphere Primitive component using par_shapes.
### View
Each option toggles a hide/view window from a list of available windows:
* **Console**: Shows Engine's logs.
* **Configuration**: Shows information about engine's modules and allows the user to edit some engine utility.
    * **Application**: Allows for frame capping (set to 0 to ignore cap) and shows framerate & milliseconds plot.
    * **Input**
    * **Window**
    * **Scene**: Allows activate/deactivate quadtree update to enclose all childs on when its added, set as static, set static as active or applying a transformation. Shows management over scene gameobjects and allows for different ways to visualize Quadtree (Bottom, Top, Top & Bottom or Full).
    * **Editor**: Allows to configure editor camera and debug drawing.
    * **Renderer 3D**: Allows to enable/disable gl flags and frustum culling.
    * **Memomy**: Shows current memory usage.
    * **Hardware**: Shows hardware specifications for running device.
    * **File System**
* **Hierarchy**: Shows distribution of Game Objects in scene.
* **Properties**: Shows information about the selected Game Object and its components.
* **Prefabs**: Allows prefab creation to duplicate scene's Game Objects.
* **Random Test**: For testing random number generation with ranges using ints and floats.
* **Texture Manager**: Shows textures loaded.
### Help
* **Open/Close ImGui Demo**: Opens/closes ImGui Demo window
* **Documentation**: opens browser to repository's wiki page
* **Download Latest**: opens browser to repository's realeses page
* **Report a Bug**: opens a browser to repository's issues page
* **About**: Shows engine info and 3rd party software.

## Libraries Used
* SDL v2.0.7
* Optick v1.2.9
* PhysFS v3.0.2
* Rapidjason v1.1.0
* LibZip v1.5.0
* ImGui v1.72b
* OpenGL v3.1.0
* Glew v2.1.0
* MathGeoLib v1.8.0
* DevIL v1.8.0
* GLSLang v1.40
* Assimp v4.0.1
* par_shapes
* gpudetect
* mmgr

## Version Notes

### v2.0
* Added **Mouse Picking** using Raycast optimized using a **Quadtree**.
* Added **Frustum Culling**. Activating camera's "_override cull_" will swap the frustum used for culling scene. Frustum culling can be disabled through configuration window. It uses Quadtree to collect intersections quicker.
* Added **Camera Aspect Ratio**. Options include: Fit Window, Square 1x1, Traditional TV 4x3, Movietone 16x9 and Personalized. Personalized aspect ratio allows user to freely set camera's aspect ratio. Window resize adapts camera and viewport.
* Added **Event System**. Used to dynamically control scene game objects and make calls between managers/modules/importers...
* Added **Skybox**.

##
![University Logo](https://www.citm.upc.edu/templates/new/img/logoCITM.png?1401879059)    
