# Whiteout Extreme

Survival Racing Game

### Play

Download the latest
[release](https://github.com/SPolton/whiteout-extreme/releases)
for Windows. Extract the zip archive and run the `exe` to play.

### Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details.

## Development Environment Setup

### Requirements

- Git LFS: `git lfs install`
- **Windows 10/11**
- **CMake 3.20+**
- **Visual Studio 2022** (recommended) or compatible C++20 compiler
- **OpenGL** Graphics library provided by the system
- **PhysX SDK 5.6.1** (Omniverse PhysX 107.3)
- **FMOD Engine 2.03.12**

### Building the Project

1. **Clone the repository**
    - SSH: `git clone git@github.com:SPolton/whiteout-extreme.git`
    - HTTPS: `git clone https://github.com/SPolton/whiteout-extreme.git`

2. **Configure with CMake**
    - Visual Studio handles the build system generation automatically.
    - If using command line, run:
    ```bash
    cmake -B build
    cmake --build build --config Debug
    cmake --build build --config Checked
    ```

### Setting Up PhysX

Follow these steps to set up NVIDIA PhysX for physics simulation:

1. **Download PhysX SDK**
    - Visit the [NVIDIA PhysX GitHub repository](https://github.com/NVIDIA-Omniverse/PhysX/releases/tag/107.3-physx-5.6.1)
    - Download and extract the source zip for `PhysX SDK 5.6.1`
    - Move the internal `physx` folder into the `libraries/physx` folder of this project.

2. **Generate PhysX Project Files**
    - Locate `physx/generate_projects.bat` and run it.
    - Choose `VC17`, the default compiler for Visual Studio 2022.
    - This creates folders in `physx/compiler`

3. **Build PhysX Libraries**
    - Locate `physx/compiler/vc17win64` or `physx/compiler/vc17win64-cpu-only`
    - Open the `PhysXSDK.sln` file in Visual Studio
    - Build the following configurations:
      - **Debug** configuration with `/MTd` (Multi-Threaded Debug static runtime)
      - **Checked** configuration with `/MT` (Multi-Threaded static runtime)

4. **Check PhysX Setup**
    - Confirm the PhysX headers exist in `libraries/physx/include/`
    - Confirm the compiled `.dll` binaries and `.lib` libraries exist in:
    ```
    physx/bin/win.x86_64.vc143.mt/debug/
    physx/bin/win.x86_64.vc143.mt/checked/
    ```

### Setting Up FMOD

Follow these steps to set up FMOD for audio integration:

1. Download [FMOD Engine 2.x](https://www.fmod.com/download#fmodengine) (Requires free account).
2. Extract the archive, or run the `exe` installer for your platform.
3. Find the FMOD install location and navigate to the `api` folder (contains `core` and `studio`).
4. Copy the `api` folder and paste into the `libraries/fmod/api` directory of this project.

### Dependencies

Manual setup needed:
- **PhysX 5.6.1** - Physics engine
- **FMOD Engine 2.03.12** - Audio engine

Most dependencies are automatically fetched by CMake:
- **GLFW 3.4** - Window and input
- **GLM 1.0.3** - Mathematics library
- **FreeType 2.14.1** - Font rendering
- **ImGui 1.92.5** - Debug panels
- **Assimp 6.0.4** - Asset import
- **rapidjson 1.1.0** - JSON parsing
- **fmt 11.2.0** - Format strings
- **vivid 3.1.0** - Color output

Bundled in project `libraries/` folder:
- [GLAD](https://glad.dav1d.de/) - OpenGL (4.6) loader
- [pl_mpeg](https://github.com/phoboslab/pl_mpeg) - MPEG playback
- [stb](https://github.com/nothings/stb) - Image helpers

# Sources

## Assets

- [common-3d-test-models](https://github.com/alecjacobson/common-3d-test-models) from Alec Jacobson
- [Dark black carbon fiber metal lines background](https://www.freepik.com/free-vector/dark-black-carbon-fiber-metal-lines-background_24243620.htm) by Starline
- [Snow Landscape](https://www.blenderkit.com/get-blenderkit/8c0aa8aa-6ac8-4047-b3b2-80474a146656/) by Antoine F.
- [Solar Textures](https://www.solarsystemscope.com/textures/) from Solar System Scope
- [Survival Guitar Backpack](https://sketchfab.com/3d-models/survival-guitar-backpack-799f8c4511f84fab8c3f12887f7e6b36) by Berk Gedik
- Survival Guitar Backpack [Modified](https://learnopengl.com/data/models/backpack.zip) from LearnOpenGL
