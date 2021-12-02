# VoxelEngine
![image](https://user-images.githubusercontent.com/16225160/144341138-77a9e83c-3f1e-4fa2-9a07-fab42decd7f1.png)


Voxel Engine with PBR rendering using Vulkan


# Features
* Entity Component System (entt)
* Simple Inverse Kinematics
* Graphics
  * PBR
  * Bloom
  * Voxelized scene for raytraced shadows, AO and specular occlusion
  * Screen Space Reflections
* Editor
  * Prefabs
  * Undo/Redo
  * Multi Scenes
  * Manipulation Guizmos (ImGuizmo)
  * Outline for object selection

# How to compile
* **Requirements**
  * [VulkanSDK](https://vulkan.lunarg.com/sdk/home)
  * Windows (only currently supported platform)
  * C++17 compiler
  * CMake 3.7+

If you are using Visual Studio 2019 install the cmake tools so you can just open the folder

Otherwise
```
git clone --recursive https://github.com/carloshgsilva/VoxelEngine
cd VEngine
mkdir build
cd build
cmake ..
```
