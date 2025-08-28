
### Example to load the SkyrimNet WebUI inside Skyrim using PrismaUI


Currently set to use F4 and hardcoded in main.cpp
`const uint32_t TOGGLE_FOCUS_KEY = 0x3E; // F4 key`

### to build:
* run `xmake build`

Currently a "debug" version is compiled. change `xmake.lua` (lines with comments of " -- debug")

An archive file should be created in the "release" folder. This can be installed as a mod.

### REQUIREMENTS:
Also needed is "PrismaUI_1_0_0.zip" from https://github.com/PrismaUI-SKSE/PrismaUI-Wiki/releases

Do NOT install this zip file.
This zip needs to be extracted first.
Install the archive from either the "INSTALL WITH MO2" or "DEBUG VERSION" folders as a normal mod.

The folder "PUT FILES IN SKYRIM FOLDER" contain files that need to go in the game root folder.

Rename (perhaps PrismUI-Ultralight) and install this folder as a normal mod in Vortex and set the type to "Engine Injector" to install in game folder.


### Notes:
in main.cpp
`PrismaUI->Focus(view, true);` is supposed to pause the game and the game is supposed to unpaude when `Unfocus` is called. However, I had issues with this so it is left False.

PrismaUI calls the html file that is in `views\index.html`. 
Currently this just loads the SkyrimNet page using "localhost:8080" inside an iframe.
It has some javascript for the hide/show functionality and an attempt at a resizing using a grab box at the corner of the window.



original README follows:

-----

# PrismaUI SKSE Plugin Template

This is a basic plugin template using PrismaUI and CommonLibSSE-NG.

> **You can download ready-to-use plugin for MO2 here: [Download PrismaUI-Example-Plugin](https://github.com/PrismaUI-SKSE/PrismaUI-Wiki/releases)**

### Requirements
* [XMake](https://xmake.io) [2.8.2+]
* C++23 Compiler (MSVC, Clang-CL)

## Getting Started
```bat
git clone --recurse-submodules <repository>
```

### Build
- To build the project, run the following command:
```bat
xmake build
```
> ***Note:*** *This will generate a `build/windows/` directory in the **project's root directory** with the build output.*

- Move `view/index.html` to your plugin folder in `<YourPluginName>/PrismaUI/views/PrismaUI-Example-UI/index.html`.

### Project Generation for Visual Studio
If you want to generate a Visual Studio project, run the following command:
```bat
xmake project -k vsxmake
```

> ***Note:*** *This will generate a `vsxmakeXXXX/` directory in the **project's root directory** using the latest version of Visual Studio installed on the system.*

### Upgrading Packages
If you want to upgrade the project's dependencies, run the following commands:
```bat
xmake repo --update
xmake require --upgrade
```

### Build Output (Optional)
If you want to redirect the build output, set one of or both of the following environment variables:

- Path to a Skyrim install folder: `XSE_TES5_GAME_PATH`

- Path to a Mod Manager mods folder: `XSE_TES5_MODS_PATH`
