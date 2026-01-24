# Whiteout Extreme

Survival Racing Game

## Development Environment Setup

### Requirements

- **Windows 10/11**
- **CMake 3.20+**
- **Visual Studio 2022** (recommended) or compatible C++20 compiler
- **PhysX SDK 5.6.1** (Omniverse PhysX 107.3)

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
    - Visit the [NVIDIA PhysX GitHub repository](https://github.com/NVIDIA-Omniverse/PhysX)
    - Download or clone `Omniverse PhysX 107.3 and PhysX SDK 5.6.1`
    - Move the `physx` folder into the root directory of this project.

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

4. **Check Physx Setup**
    - Confirm the PhysX headers exist in `physx/include/`
    - Confirm the compiled `.dll` binaries and `.lib` libraries exist in:
    ```
    physx/bin/win.x86_64.vc143.mt/debug/
    physx/bin/win.x86_64.vc143.mt/checked/
    ```

### Dependencies

Most dependencies are automatically fetched by CMake:
- **GLFW 3.4** - Window and input management (fetched automatically)
- **GLM 1.0.3** - Mathematics library (fetched automatically)
- **GLAD** - OpenGL loader (included in `libraries/`)
- **ImGui 1.92.5** - UI library (fetched automatically)
- **PhysX 5.6.1** - Physics engine (manual setup required)

### Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details.
