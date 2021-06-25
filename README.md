# Red Eye ![logo](https://github.com/juliamauri/RedEye-Engine/blob/master/docs/favicon-32x32.png?raw=true) Engine [![Build status](https://ci.appveyor.com/api/projects/status/swrp9sgx89yxl493?svg=true)](https://ci.appveyor.com/project/cumus/redeye-engine)

Red Eye is a 3D Game Engine written in C++.

What’s special about it? The code was written from scratch using a total of 15 C++ libraries. We started by building a geometry loader that could render the scene with simple controls for the camera. Once the base was solid we moved on to optimize the rendering process (frustum culling and quadtrees) and resource management. For the finishing touches we:
* Added shader pipeline with integrated editor so that shaders can be modified, compiled and updated instantly.
* Changed quadtree for a better space partitioning algorithm: Dynamic Bound Box Tree; the same Overwatch uses.
* Added Unity-like resource management with binary save/load for all the engines resources.
* Added particle pipeline with physics and graphics customization.

Future plans: improve the engine by adding a 3d physics particle simulation and rendering pipeline.

## Links:
* Repository [Github](https://github.com/juliamauri/RedEye-Engine)
* Webpage: https://redeye-engine.es/
* Authors: [Julià Mauri Costa](https://github.com/juliamauri) & [Rubén Sardón](https://github.com/cumus)
* Tutor 1: [Ricard Pillosu](https://github.com/d0n3val) (9/2018-2/2019)
* Tutor 2: [Marc Garrigó](https://github.com/markitus18) (9/2019-2/2020)
* Tutor 3: [Lasse Löpfe](https://www.linkedin.com/in/lasse-loepfe) (2/2021-6/2021)
* University: [CITM UPC](https://www.citm.upc.edu/)
* License: [GNU General Public License v3.0](https://github.com/juliamauri/RedEye-Engine/blob/master/LICENSE)

## Latest Version Notes

### Release v5.0 Particle Pipeline

 * Added module phyisics: updates particle manager using fixed, timed-steps or engine par delta time.
 * Added particle emitter: stores simulation data and handles particle iteration
 * Added particle emitter gameobject component: places referenced particle emitter resource in the scene.
 * Added new particle resources:
    * Emission: stores spawning and phyical properties
    * Rendering: stores orientation, graphics and lighting properties
    * Emitter: stores an emission resource and rendering resource
 * Added emitter workspace: editor window that comfortably allows modifying all the different properties of a simulation. Includes a viewport with its own camera and controls.
 * Improved GPU memory usage. Increased maximum supported rendering lights from 64 to 203 for light components and up to 508 for particle point lights.
 * Added primitive resource manager: reference counts loaded primitives
 * Added physics and rendering profiling methods to measure each system's performance.

## User Actions
### Shortcuts
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

## User Interface
### File
* **New Scene**: Empty the scene with non scene resource linked.
* **Save Scene**: Save current scene.
* **Exit**: closes engine.
### Edit
* **Undo**: Undo transformations (CTRL + Z).
* **Redo**: Redo transformations (CTRL + Y).
### Assets
* **Create**:
    * **Material**:  Creates a new material.
    * **Shader**: Creates a new shader with the script editor or scripts from assets.
    * **Skybox**:  Creates a new skybox.
    * **Water Resources**: Creates a water resource(component independent). 
### Gameobjects
* **Primitive**: List that you can select what primitive you want using par_shapes.
    * **Grid**
    * **Cube**
    * **Dodecahedron**
    * **Tetrahedron**
    * **Octohedron**
    * **Icosahedron**
    * **Plane**
    * **Sphere**
    * **Cylinder**
    * **HemiSphere**
    * **Torus**
    * **Trefoil Knot**
    * **Rock**
* **Camera**: creates a Game Object with a Camera component.
* **Light**: creates a Game Object with a Light component.
* **Max Lights**: creates a Game Object with a total childs from max light (64, debug purposes).
* **Water**: creates a Game Object with a Water component.
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
* **Wwise**: You can send events to Wwise for reproduce your bank.
* **Tools**:
    * **Random Test**: For testing random number generation with ranges using ints, floats and unsigned long long (with bitwise operator).
    * **Transforms Debug Info**: for showing all transforms in thr current scene.
### Help
* **Open/Close ImGui Demo**: Opens/closes ImGui Demo window
* **Documentation**: opens browser tab to repository's wiki page
* **Download Latest**: opens browser tab to repository's releases page
* **Report a Bug**: opens a browser tab to repository's issues page
* **About**: Shows engine info and 3rd party software.

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
It detects any changes of assets folder and imports (non-recursive). Use an own base Path struct with directory, file and meta. The directory will detect the changes and return a stack of changes to do. At start of engine this method will process all assets folder; after, it enters at end of every frame with the extra seconds before the frame ends.
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
### Profiling
Started using Optick to profile engine's methods. Adapted calls to support custom profiling pipeline. Internal profiling registers function calls and tick intervals and can deploy registers to a .json file. We can feed the output to Tableau and graphically analize performance metrics.
### Particle Pipeline
Physics module supports running particle simulations inside a Particle Manager. Particle physics, spawning, rendering behaviours can be edited through the emitter workspace window.
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

## Previous Version Notes

### Release v4.0 ***Big Chunky Update***
* Graphics:
    * Added and implemented **Deferred Light Shading**!
        * New render pipeline can toggle lighting modes.
        * **Added Component Light**!
        * Support for material Shininess value.
    * Added **Commponent Water**!
        * Implemented **dynamic foam** on water!
          * New shader's internal values are parsed and uploaded to the shader. You may now request the depth texture generated each frame.
        * Because of new render pipeline, we configured a water component that works automatically with and without deferring.
        * Implemented procedure automatically generate your own water resource. You may select if you want deferred or not (**engine scene**, everything we use on the engine you can implemented it step by step with our editor tools!).
* **Performance**:
    * Implemented Gameobject's and components' Hash Table (RE_GOManager class). 
        * **Hash Table removes using new's, delete's and UUID** (windows). Now using UID (unsigned long long) generated with our random generator (playing with **bitwise operators**) to reference any gamepbject or component.
        * Upgraded Hash Table array's to **dynamic arrays**, increasing array size when full.
        * Implemented RE_GOManager on scene and gameobjects resources (Model, Scene and Prefab).
        * **New serialization** based on Hash Table direct memory copying.
        * Using Hash Tables allows **x3 faster iteration** in scene elements. All data is neatly stored together and removes using pointers to access all elements.
    * Code CleanUp:
        * **Removing warning**s.
        * Implementing **C++ casts** (dynamic, static, const and reinterpret).
        * **Zero recursivity** methods.
        * **Removing App pointer**, welcome static values.
    * Fixed some OpenGL errors and warnings. We found them thanks to adding new debug log polling from opengl.
* Resources:
    * Implemented **erase resources**. It updates other resources with dependencies.
    * Visual upgrade on panel assets.
    * Fixed thumbnails; although an issue remains where some gameobjects and material thumbnails don't render properly.
* Usability
    * Implemented **destroy GameObject**!
    * Added create prefab from any gameobject of hierarchy.
    * Added **Gizmo** (ImGuizmo).
    * Added and implemented **Commands!** You can do and redo all transformation changes!
    * New window monitoring all scene transforms (debug pruposes).
    * Added **OpenGL Debug Output**. Now automatcally send logs to console!
    * "New Scene" and "Save Scene" Buttons. PopUp window opens before closing if current scene changes haven't been save.
* Implemented all primitives from **par_shapes.h**
* Added functional Grid Component!
* Added and implemented new AudioModule with **Wwise**!
    * You can configure the folder of output from wwise editor inside of engine folder for **hot reloading** new changes inmediatly!
    * Added new audio module panel which to send calls to Wwise.
* Added internal custom profiling. Key method calls are recorded when starting profiling session. Session may be paused and resumed later on. Having recorded at least a method displays a button to save session to file and another to clear them.
* Current WIP issues:
    * [Thumbnail camera not positioning correctly for rendered icons](https://github.com/juliamauri/RedEye-Engine/issues/3)
    * [Primitive's AABBs do not enclose geometry](https://github.com/juliamauri/RedEye-Engine/issues/2)
    * [Thumbnail not rendering models and crashing](https://github.com/juliamauri/RedEye-Engine/issues/1)

### Release v3.2
* Added and implemented **EASTL** and EA libraries' dependencies. Good bye STD.
* Fixed **Dynamic AABB Tree** crash.

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
