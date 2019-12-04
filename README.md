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
### Supported Drag & Drop
* Any **File, Folder and .zip** dropped will copy to assets on the current directory of assets panel and the Read Assets Changes from filesystem will detect and import automatically.

## Other Systems
### Camera Manager & Target Aspect Ratio
Cameras are handled through the new CameraManager class. Using static functions allows for clean getting of cameras anywhere in code. Using the also new events system, the cameras get the renderer's gl_Viewport updated given any window resizes.
### Quadtree
Using the events system, scene can control where all root gameobjects are. If any gameobjects exit the Quadtree boundaries after transformation, they get stored away from the quadtree (Pop) in a list with the rest of static objects outside the quadtree. If a gameobject re-enters the quadtre boundaries, its re-added to the quadtree (Push).
### SkyBox
Default skybox loads on start. Using glDepthFunc(GL_LEQUAL) instead of glDepthFunc(GL_LESS) and rendered last, the skybox always draws the furthest away. For passing the 6 different textures,GL_TEXTURE_CUBE_MAP binds on draw call.
### Console & Error Handler
Engine logs all startup and importing procedures and can be accessed through the console window. This window contains a menu to filter logs shown by category and by file. Upon having to log an error report, a Pop Up appears showing the error description and the solution opted for. Additional logs and warnings about this procedure can be seen inside the same Pop Up.
### Bounding Boxes
Game Objects have local and global bounding boxes and are updated when applying transformations. Parent Game Objects enclose their child's bounding boxes but don't reset on applying transformations to a child. This step is manual and a Button for reseting them will appear in the scene's Configuration Window. Reset is automatically applied on importing new geometry to scene.

When focusing a Game Object, the camera calculates the minimum distance from the geometry which encloses it using its global bounding box.
## Resources
Own imported resources that contains information from asset and library file. The own generated resources, the file on assets will be serialized with Rapidjson file (user redeable file) and binary at library.
Model (from .fbx), texture, skybox (contain textures), material (contain textures and shader), scene, prefab and shader.
## Meta files
All importabe files will generate a .meta file that indicats it's imported and detected by the read assets changes at init, evading to import another time. Can contain import settings of any resource.
### Materials
When importing geometry, all materials used are saved to /Assets/Materials as a Rapidjson file. Any textures contained by them are saved to /Library/Images.
### Shaders
All geometry is rendered through shaders except bounding boxes. Just using for now 2 shaders that draw geometry with a given diffuse texture or a diffuse color.
### MD5 & Resource Referencing
All resources use an MD5 generated strings to reference each other and to quickly check if they are duplicated when importing. How it works:
We put all asset data of a resource in a buffer and objtain the MD5. The name of resource on library will be the md5 without any extension.
### GameObjects/Components with Resources Serialization
When create a new scene or prefab resource, will be saved on assets as JSON and at library as Binary. Before the gameobject seraialization the importer will detect all unique resources from gameobject and mapped and serialized it.
### Read Assets Changes
It detects any changes of assets folder and imports (no recursion). Use an own base Path struct with directory, file and meta. The directory will detect the changes and return a stack of changes to do. At start of engine this method will process all assets folder, after it enters at end of every frame with the extra seconds before the frame ends.
### ZIP
When droping and .fbx file, its path will vary depending on which platform de engine is running on and there must be a 
Importing .fbx files may trigger errors while checking the materials' texture paths. These paths vary depending on which platform de engine is running and there must be a conversion if needed. When importing a .zip file containing all resources allows for LibZip and PhysFS to create a virtual directory through which access is already handled. This system allows cross-platform imports through .zip files.

## User Interface
### File
* **Exit**: closes engine.
### Create
* **Plane**: creates a Game Object with a Plane Primitive component using par_shapes. It can be converted into mesh.
* **Cube**: creates a Game Object with a Cube Primitive component using par_shapes.
* **Sphere**: creates a Game Object with a Sphere Primitive component using par_shapes.
* **Camera**: creates a Game Object with a Camera component.
### View
Each option toggles a hide/view window from a list of available windows:
* **Console**: Shows Engine's logs.
* **Configuration**: Shows information about engine's modules and allows the user to edit some engine utility.
    * **Application**: Allows for frame capping (set to 0 to ignore cap) and shows framerate & milliseconds plot.
    * **Input**
    * **Window**
    * **Scene**: Shows management over scene gameobjects and allows for different ways to visualize Quadtree (Bottom, Top, Top & Bottom or Full).
    * **Editor**: Allows to configure editor camera and debug drawing.
    * **Renderer 3D**: Allows to enable/disable gl flags and frustum culling.
    * **Memomy**: Shows current memory usage.
    * **Hardware**: Shows hardware specifications for running device.
    * **File System**: Shows paths info.
* **Hierarchy**: Shows distribution of Game Objects in scene.
* **Properties**: Shows information about the selected Game Object, its components and resources.
* **Assets Panel**: Shows all resources from the assets folder, like explorer.exe.
* **Prefabs**: Allows prefab creation to duplicate scene's Game Objects.
* **Random Test**: For testing random number generation with ranges using ints and floats.
* **Texture Manager**: Shows textures loaded.
* **Material Editor**: Craete a new material.
* **Shader Editor**: Create a new shader with the script editor or scripts from assets.
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

### PreRelease v3.0
* Added **Default Material and Shader resources** for a little start.
* Added **Grid and Plane primitive component**. You can make your own materials.
    * Plane can **convert to mesh**, for change it's material.
* Added **Material Editor Window**. You can make your own materials.
* Added **Shader Editor Window**. You can compile your own shader and select/edit or create shader files.
* Complete **New resource system**. The biggest internal change.
    * Added and implemented **New resource class** to manage all files from assets. Contains information about the resource and  it's resource can be identified by the meta referenced all importable assets. All information on assets is redeable and the new resources created by the user it's serialized in JSON and all the files on library the name is md5 and it's serialized by binary.
        * Added **Model, Texture and SkyBox resources** with import settings.
        * Added **Scene and Prefab resources** for gameobject serialization.
        * Added **Material resource** compartible with model importer.
        * Added **Shader resource** for customizable shaders.
    * Added **Assets Panel**. Shows all resources from assets folders and you can drop any type of files/folder/zip, the engine will copy and detect the importable files. You can select the resource and watch the resource information on inspector panel.
    * Added **Read Asset Changes** for detecting any dropped file and import. Own virtual filesystem of assets directory.
    * Added **New resource manager** that handles all recources with **reference counting** for memory management. 
    
##
![University Logo](https://www.citm.upc.edu/templates/new/img/logoCITM.png?1401879059)    
