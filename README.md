# VoxCore Game Engine

**VoxCore Game Engine** is a work-in-progress 3D voxel-based game engine built using **Vulkan**, **SDL3**, and **ImGui**. It is designed to be modular, lightweight, and highly extensible for voxel-style games and simulations.

> ⚠️ This engine is currently under active development. Many features are incomplete or experimental. Breaking changes are expected.

> 🎓 This engine is a personal learning project focused on exploring the Vulkan API and game engine architecture. It is not intended for production use, but serves as an experimental and educational platform.

## Branches
- **main** – Stable branch. Contains the latest tested and (mostly) stable code. Recommended for users who want to try the engine.
- **dev** – Development branch. Active development happens here. It may be broken, untested, or not even compile at times.

## Features (WIP)
- 🧱 Custom voxel renderer powered by Vulkan
- 🖥️ Integrated **ImGui-based** UI system as the main graphics framework
- 🖼️ UI layout system using XML files (similar to WPF)
- 🏗️ ACS-style architecture (Actor-Component-System, similar to Unreal Engine)
    - **Systems** link multiple components with shared logic
- 💬 **Convars and in-game console** inspired by the Source Engine
- 🚀 Flexible graphics pipeline system for rendering customization
- 🔧 Planned support for scripting, world generation, physics, and more...

## Tech Stack
- [Vulkan](https://www.vulkan.org/) – low-level graphics API
- [SDL3](https://github.com/libsdl-org/SDL) – cross-platform window and input handling
- [ImGui](https://github.com/ocornut/imgui) – immediate mode GUI and main graphics framework
- [GLM](https://github.com/g-truc/glm) – math library for graphics and transformations
- [GoogleTest](https://github.com/google/googletest) – unit testing framework
- [stb](https://github.com/nothings/stb) – image, font, and general-purpose utilities
- [pugixml](https://github.com/zeux/pugixml) – XML parsing library (used for UI layouts)
- [nlohmann/json](https://github.com/nlohmann/json) – JSON parsing library
- C++23

## Projects Using This Engine

| Project Name | Description            | Status     |
|--------------|------------------------|------------|
| [VoxCraft](https://github.com/idk-this/VoxCraft) | Simple Minecraft clone | In develop |

## Build Instructions
(Coming soon – build system, dependencies, and steps...)

## License

This project is licensed under the [GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.en.html).
