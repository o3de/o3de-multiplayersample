# Settings Screen

The settings screen contains a set of controls for modifying various options in O3DE. This screen demonstrates how to expose settings from O3DE to the user and how to persist those settings to a file. 

## User Settings
### Graphics API

O3DE supports running with multiple different graphics APIs, depending on the platform. The full set of APIs currently supported are the following:

* "null" - A null renderer, nothing is drawn to the screen
* "dx12" - The DirectX 12 API (Windows only)
* "vulkan" - The Vulkan API (all platforms)
* "metal" - The Metal API (iOS / Mac platforms only)

The different APIs use different OS-level and driver-level code, which can result in performance differences based on a user's specific operating system, graphics card, and graphics drivers.

Normally the API is selected through a command-line argument, such as `-rhi="null"`. The setting on the settings screen provides the ability to specify the default API to use on subsequent runs of the launcher.

The implementation of this setting is more complex than the other settings because the user setting needs to be read in after enough of the engine has initialized to be able to load in settings from project-specific code, but before the renderer itself has initialized; the API choice cannot be changed once the renderer is initialized. This timing occurs during the construction of the project's Module class. This setting is implemented by modifying a settings registry key that controls what priority the renderer should use to select an API.

### Texture Quality

The Texture Quality setting controls the "mip bias", which is the highest mip level to load for each streamed texture. A value of `0` is the highest-resolution mipmap, `1` is the second-highest mipmap, and so on. This value exposes the tradeoff between quality and memory - the highest-resolution textures look the best but use the most VRAM. The user can choose to lower the quality and lower the VRAM usage, which can help with performance.

In O3DE, the mip bias is exposed through a cvar, a settings registry key, and an API. The settings screen uses the API to directly modify the setting.

### Audio Volume

The Audio Volume setting controls the master volume for the entire audio system, from 0 (min) to 100 (max). The audio system has data-driven volume channels; each project's audio setup may have a different set of more granular volume controls, such as for music, sound effects, voice, and ambient noise. This setting implements a master volume control, but any of the more granular controls would be implemented in a similar way.

Volume controls for the audio system are exposed through an API, and not through any cvar or settings registry keys. The settings screen uses the API directly to modify the value.

### Fullscreen

The Fullscreen setting controls whether the launcher is running in a window on the desktop or in fullscreen mode.

The fullscreen/windowed mode selection is controlled by the `r_fullscreen` cvar. The settings screen demonstrates how to set the cvar from a user setting.

## Modifying and extending the settings screen

See the following source files for the settings screen implementation:
* https://github.com/o3de/o3de-multiplayersample/blob/development/Gem/Code/Source/UserSettings/MultiplayerSampleUserSettings.cpp
* https://github.com/o3de/o3de-multiplayersample/blob/development/Gem/Code/Source/Components/UI/UiSettingsComponent.cpp

The `MultiplayerSampleUserSettings.cpp` file contains the backend logic for loading and saving the user settings and applying the settings to the engine. The default values for each setting can be found here as well. More settings could be exposed and implemented here.

The `UiSettingsComponent.cpp` file contains the UX logic for turning the setting values into user-friendly names and toggle controls. The names and specific choices of which values to expose can be found here. For example, the Master Volume control exposes the volume in increments of 10, but it could be modified to expose more volume values to increment it by 5 or by 1. Similarly, the Graphics API control could be modified to only expose APIs that exist for that platform.
