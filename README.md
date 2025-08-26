# GD Speak

GD Speak is a GDExtension that acts as a bridge between Godot and [eSpeak-NG](https://github.com/espeak-ng/espeak-ng). It handles text-to-speech synthesis and delivers audio data to Godot in a format that can played directly from memory.

## Changes from Original Repository

Originally forked from [eSpeak-NG-for-Godot-4](https://github.com/soykhaler/eSpeak-NG-for-Godot-4) with intent to fix errors I was getting in Godot when trying to use the plugin on Windows. Big thanks to [@soykhaler](https://github.com/soykhaler) for the original code, this saved me significant time on my personal Godot project 😊

- Established Windows compatibility
	- Note this repo *only* contains the files needed for building and using the gdextension on Windows
	- Rebuilt [eSpeak-NG 1.52](https://github.com/espeak-ng/espeak-ng/releases/tag/1.52.0) with MSVC to add `libespeak-ng.dll` (previously missing dependency of `libespeakgodotextension.dll`)
        - I believe this is the only change I needed for [eSpeak-NG-for-Godot-4](https://github.com/soykhaler/eSpeak-NG-for-Godot-4) to work on Windows
	- Rebuilt the GDExtension for a mutually compatible `libespeakgodotextension.dll`
- Removed the demo Godot project from the original repo
- File restructuring for organization into a Godot addon folder (`addons/gd-speak`)
    - Allows for simply copying `addons/gd-speak` to your project's `addons` folder to use in other Godot 4.4 projects
- Significant changes to the original `eSpeakNode`, made for personal preference and usage in my Godot project
    - Renamed `ESpeakNode` to `ESpeaker`
    - Print statements now require `ESpeaker.debug_print` to be enabled per `ESpeaker` instance (`false` by default)

Thanks again to [@soykhaler](https://github.com/soykhaler) for the original project design and repository!

## 1. How to Update eSpeak-NG Changes and Recompile the GDExtension

If you modify the eSpeak-NG source code (within `espeak-ng-src`, which is not included here but can be cloned from [eSpeak-NG 1.52](https://github.com/espeak-ng/espeak-ng/releases/tag/1.52.0)) or the GDExtension code (`src`), you need to recompile the GDExtension for changes to reflect in your Godot project.

Follow these steps:

1.  **Navigate to the GDExtension directory:**
    ```bash
    cd gd-speak
    ```

2.  **Recompile the GDExtension using SCons:**
    ```bash
    scons platform=windows
    ```
    This command rebuilds the `libespeakgodotextension.dll` library located in `addons/gd-speak/bin/windows/`. It also adds some extra files `libespeakgodotextension.dll` and `libespeakgodotextension.lib` in the same directory. These can be deleted as they are not needed in Godot.

3.  **Restart the Godot editor:** After recompilation, it's crucial to close and reopen the Godot editor for it to load the new library version. If you only run the scene, Godot might still use the older version in memory.

## 2. Does eSpeak-NG Need to Be Installed on the System?

**No, eSpeak-NG does not need to be installed on your operating system.**

The compiled GDExtension `libespeakgodotextension.dll` depends on the `libespeak-ng.dll` library and its data files (`addons/gd-speak/espeak-ng-data`). If the data folder is missing, eSpeak-NG has a fallback to look for the data files on your system. This allows the extension to work anyways if you have previously installed eSpeak-NG with its `.msi` installer (see release files for [eSpeak-NG 1.52](https://github.com/espeak-ng/espeak-ng/releases/tag/1.52.0)). Take note of this, as users of your Godot project may not have eSpeak-NG installed on their system. This can result in the gdextension working fine for you, but not for your users. Make sure the data exists at `addons/gd-speak/espeak-ng-data` to avoid this issue.

As long as `addons/gd-speak/espeak-ng-data` properly exists, your project is **self-contained** regarding eSpeak-NG. You can distribute your Godot project, and eSpeak-NG will work without requiring the end-user to have eSpeak-NG pre-installed on their system.

## 3. How the Code Works (High-Level)

The eSpeak-NG integration in your Godot project is achieved via a C++ **GDExtension**. Here's a summary of how the parts interact:

*   **`espeak.gdextension` (Godot Configuration File):**
    *   This file tells Godot that a GDExtension exists and where to find the compiled library (`libespeakgodotextension.so`).

*   **`libespeakgodotextension.so` (Compiled Library - C++):**
    *   It's the core of the integration. It contains the C++ code that communicates directly with the `espeak-ng` library.
    *   It defines an `ESpeaker` class that inherits from Godot's `Node`, making it accessible in your Godot scenes.

*   **`ESpeaker` (C++ Class):**
    *   **Initialization (`ESpeaker::ESpeaker()`):** When an `ESpeaker` instance is created in Godot, its constructor initializes the `espeak-ng` library. It tells `espeak-ng` where to find its data files (`espeak-ng-data`) and sets up a callback to receive audio data.
    *   **Speech Synthesis (`ESpeaker::synthesize(text)`):** This method takes a Godot `String` (the text to speak), converts it to a format `espeak-ng` understands, and asks `espeak-ng` to synthesize the audio.
        *   `espeak-ng` doesn't return audio immediately; instead, it repeatedly calls a callback function (`synth_callback`) as it generates audio fragments.
        *   The `synthesize` method waits for `espeak-ng` to finish generating all audio (`espeak_Synchronize()`) and collects all audio fragments into a `PackedByteArray`.
        *   **Important:** This method returns *only raw audio data* (16-bit PCM, mono, 22050 Hz), without the WAV header.
    *   **Language Change (`ESpeaker::set_language(lang_code)`):** This method takes a language code (e.g., "en", "es") and tells `espeak-ng` to change the voice used for synthesis.

*   **`main.gd` (GDScript Script):**
    *   This script is the controller for your Godot UI.
    *   It gets references to `LineEdit`, `AudioStreamPlayer`, and your `ESpeaker` nodes.
    *   When the user enters text and presses the "speak" button:
        *   It calls `espeak_node.synthesize(text)` to get the raw audio data.
        *   It creates a Godot `AudioStreamWAV`.
        *   **It explicitly configures the `AudioStreamWAV` format** (16-bit, 22050 Hz, mono) so Godot knows how to correctly interpret the raw audio data received from the GDExtension.
        *   It assigns the audio data to the `AudioStreamWAV` and plays it via the `AudioStreamPlayer`.
    *   When the user interacts with the `CheckButton` (`SpanishToggle`):
        *   It calls `espeak_node.set_language()` with the appropriate language code ("es" or "en") to change the eSpeak-NG voice.