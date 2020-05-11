# Red Eye Engine [![Build status](https://ci.appveyor.com/api/projects/status/swrp9sgx89yxl493?svg=true)](https://ci.appveyor.com/project/cumus/redeye-engine)

Red Eye is a 3D Game Engine written in C++ as an assignment for a university course.

What’s special about it? The code was written from scratch using a total of 15 C++ libraries. We started by building a geometry loader that could render the scene with simple controls for the camera. Once the base was solid we moved on to optimize the rendering process (frustum culling and quadtrees) and resource management. For the finishing touches we:
* Added shader pipeline with integrated editor so that shaders can be modified, compiled and updated instantly.
* Changed quadtree for a better space partitioning algorithm: Dynamic Bound Box Tree; the same Overwatch uses.
* Added Unity-like resource management with binary save/load for all the engines resources.

## Links:
* Repository [Github](https://github.com/juliamauri/RedEye-Engine)
* Webpage: https://redeye-engine.es/
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
### Shaders
Shaders are used through the shader manager. This manager contains the large opengl calls making rendering code easier to read. The programs are parsed to identify uniforms and be able to modify then using the editor.
### GL Cache
This subsystem stores recent gl calls in order to evade repeating code. Now that we render more stuff we need to make sure we don’t overflow the graphics card.
### Shader Editor
Using ImGui’s text editor shaders can be edited in-editor. With File system reading changes on the extra milliseconds of each frame, any changes are detected and trigger the shader (or another resource) to be updated.
### Thumbnails
In order to view the Assets more easily than just text, FBOs are used to render the resource into a texture. Materials, textures, scenes, prefabs and models have the rendered textures for thumbnails.
### Water Shader
The sample shader we made has several uniforms to modify behavior and color for the wave. It's a Gerstner wave shader.Try changing the values to see what happens!
### SkyBox
Default skybox loads on start. Using glDepthFunc(GL_LEQUAL) instead of glDepthFunc(GL_LESS) and rendered last, the skybox always draws the furthest away. For passing the 6 different textures,GL_TEXTURE_CUBE_MAP binds on draw call. Geometry used to render is a primitive sphere (smoother than squared).
### Dynamic AABB Tree
Scene module manages 2 Dynamic AABB Trees: one for static gameobjects and another for dynamic ones. Using the events system, scene gets broadcasted gameobjects' changes (active/inactive, static/dynamic, new child, remove child, transform modified...) and updates the trees acordingly to optimize **Frustum Culling** and **Mouse Picking**. Previously a quadtree was used but keeping it updated each frame was too costly. Dynamic Trees allow for fast element Push & Pop. [Reference used](http://box2d.org/files/GDC2019/ErinCatto_DynamicBVH_GDC2019.pdf).
### Bounding Boxes
Game Objects have local and global bounding boxes and are updated when applying transformations. Used for Dynamic Trees and when focusing the camera on a gameobject. The camera calculates the minimum distance from the geometry which encloses it and its childs' geometry using their global bounding box.
### Resources
Own imported resources that contains information from asset and library file. The own generated resources, the file on assets will be serialized with Rapidjson file (user redeable file) and binary at library.
Model (from .fbx), texture, skybox (contain textures), material (contain textures and shader), scene, prefab and shader.
### MD5 & Resource Referencing
All resources use an MD5 generated strings to reference each other and to quickly check if they are duplicated when importing. How it works:
We put all asset data of a resource in a buffer and objtain the MD5. The name of resource on library will be the md5 without any extension.
### GameObjects/Components with Resources Serialization
When create a new scene or prefab resource, will be saved on assets as JSON and at library as Binary. Before the gameobject seraialization the importer will detect all unique resources from gameobject and mapped and serialized it.
### Read Assets Changes
It detects any changes of assets folder and imports (no recursion). Use an own base Path struct with directory, file and meta. The directory will detect the changes and return a stack of changes to do. At start of engine this method will process all assets folder, after it enters at end of every frame with the extra seconds before the frame ends.
### Meta files
All importabe files will generate a .meta file that indicats it's imported and detected by the read assets changes at init, evading to import another time. Can contain import settings of any resource.
### Materials
When importing geometry, all materials used are saved to /Assets/Materials as a Rapidjson file. Any textures contained by them are saved to /Library/Images.
### Camera Manager & Target Aspect Ratio
Cameras are handled through the new CameraManager class. Using static functions allows for clean getting of cameras anywhere in code. Using the also new events system, the cameras get the renderer's gl_Viewport updated given any window resizes.
### Console & Error Handler
Engine logs all startup and importing procedures and can be accessed through the console window. This window contains a menu to filter logs shown by category and by file. Upon having to log an error report, a Pop Up appears showing the error description and the solution opted for. Additional logs and warnings about this procedure can be seen inside the same Pop Up.
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
* EABase v2.09.05
* EASTL v3.16.05
* EAStdC v1.26.03
* EAAssert v1.5.8
* EAThread v1.32.09
* SDL v2.0.7
* Optick v1.2.9
* PhysFS v3.0.2
* Rapidjason v1.1.0
* LibZip v1.5.0
* ImGui v1.74
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

### Release v3.2
* Added and implemented **EASTL** and dependence EA libraries. Bye STD.
* Fixed **Dynamic AABB Tree** crash

### Release v3.1
* Added **shader parsing** for uniform values.
* Added **Dynamic AABB Tree**
* Added **Gerstner Wave Shader**
* Added **Spherical Skybox**
* Added **Thumbnails** for assets panel


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

### Release v2.0
* Added **Scene Serialization** on Play. Scene's root is saved as prefab until stop resets scene from prefab.
* Added **Mouse Picking** using Raycast optimized using a **Quadtree**.
* Added **Frustum Culling**. Activating camera's "override cull" will swap the frustum used for culling scene. Frustum culling can be disabled through configuration window. It uses Quadtree to collect intersections quicker.
* Added **Camera Aspect Ratio**. Options include: Fit Window, Square 1x1, Traditional TV 4x3, Movietone 16x9 and Personalized. Personalized aspect ratio allows user to freely set camera's aspect ratio. Window resize adapts camera and viewport.
* Added Event System. Used to dynamically control scene game objects and make calls between managers/modules/importers...
* Added **Skybox**.
    
##
![University Logo](https://www.citm.upc.edu/templates/new/img/logoCITM.png?1401879059)    
