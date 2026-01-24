# Whiteout Extreme

Survival Racing Game

## Development Environment Setup

### Requirements

- **Windows 10/11**
- **CMake 3.20+**
- **Visual Studio 2022** (recommended) or compatible C++20 compiler

### Building the Project

1. **Clone the repository**
    - HTTPS: `git clone https://github.com/SPolton/racing-game.git`
    - SSH: `git clone git@github.com:SPolton/racing-game.git`

2. **Configure with CMake**
    - Visual Studio handles the build system generation automatically.

### Dependencies

Most dependencies are automatically fetched by CMake:
- **GLFW 3.4** - Window and input management (fetched automatically)
- **GLM 1.0.3** - Mathematics library (fetched automatically)
- **GLAD** - OpenGL loader (included in `libraries/`)
- **ImGui 1.92.5** - UI library (fetched automatically)
- **PhysX 5.6.1** - Physics engine (manual setup required)

### Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details.
