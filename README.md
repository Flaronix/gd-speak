# eSpeak-NG for Godot

- Forked from [eSpeak-NG-for-Godot-4](https://github.com/soykhaler/eSpeak-NG-for-Godot-4)
	- Thanks to [soykhaler](https://github.com/soykhaler) for the original code!
	- Tested on Windows; Orignal project has complete mapping for Linux only
	- (WIP) Removed the demo project from the original repo
- (WIP) File restructuring for organization and for usage as a Godot plugin
- (WIP) Significant changes to the original `eSpeakNode`, made for personal preference and usage in my Godot project
- (WIP) Uses a rebuilt [eSpeak-NG 1.52](https://github.com/espeak-ng/espeak-ng/releases/tag/1.52.0)

## 1. How to Update eSpeak-NG Changes and Recompile the GDExtension

If you modify the eSpeak-NG source code (within `gextension_espeak/espeak-ng-src`) or the GDExtension code (`gextension_espeak/src`), you need to recompile the GDExtension for changes to reflect in your Godot project.

Follow these steps:

1.  **Navigate to the GDExtension directory:**
    ```bash
    cd /home/soykhaler/espeakgodot/gextension_espeak
    ```

2.  **Recompile the GDExtension using SCons:**
    ```bash
    scons
    ```
    This command rebuilds the `libespeakgodotextension.so` library located in `espeakgodot/bin/`.

3.  **Restart the Godot editor:** After recompilation, it's crucial to close and reopen the Godot editor for it to load the new library version. If you only run the scene, Godot might still use the older version in memory.

## 2. Does eSpeak-NG Need to Be Installed on the System?

**No, eSpeak-NG does not need to be installed on your operating system.**

The compiled GDExtension (`libespeakgodotextension.so`) includes the `libespeak-ng.so` library and its data files (`espeak-ng-data`) embedded or locally within your Godot project (specifically in the `espeakgodot/bin/` folder).

This means your project is **self-contained** regarding eSpeak-NG. You can distribute your Godot project, and eSpeak-NG will work without requiring the end-user to have eSpeak-NG pre-installed on their system.

## 3. How the Code Works (High-Level)

The eSpeak-NG integration in your Godot project is achieved via a C++ **GDExtension**. Here's a summary of how the parts interact:

*   **`espeak.gdextension` (Godot Configuration File):**
    *   This file tells Godot that a GDExtension exists and where to find the compiled library (`libespeakgodotextension.so`).

*   **`libespeakgodotextension.so` (Compiled Library - C++):**
    *   It's the core of the integration. It contains the C++ code that communicates directly with the `espeak-ng` library.
    *   It defines an `ESpeakNode` class that inherits from Godot's `Node`, making it accessible in your Godot scenes.

*   **`ESpeakNode` (C++ Class):**
    *   **Initialization (`ESpeakNode::ESpeakNode()`):** When an `ESpeakNode` instance is created in Godot, its constructor initializes the `espeak-ng` library. It tells `espeak-ng` where to find its data files (`espeak-ng-data`) and sets up a callback to receive audio data.
    *   **Speech Synthesis (`ESpeakNode::synthesize(text)`):** This method takes a Godot `String` (the text to speak), converts it to a format `espeak-ng` understands, and asks `espeak-ng` to synthesize the audio.
        *   `espeak-ng` doesn't return audio immediately; instead, it repeatedly calls a callback function (`synth_callback`) as it generates audio fragments.
        *   The `synthesize` method waits for `espeak-ng` to finish generating all audio (`espeak_Synchronize()`) and collects all audio fragments into a `PackedByteArray`.
        *   **Important:** This method returns *only raw audio data* (16-bit PCM, mono, 22050 Hz), without the WAV header.
    *   **Language Change (`ESpeakNode::set_language(lang_code)`):** This method takes a language code (e.g., "en", "es") and tells `espeak-ng` to change the voice used for synthesis.

*   **`main.gd` (GDScript Script):**
    *   This script is the controller for your Godot UI.
    *   It gets references to `LineEdit`, `AudioStreamPlayer`, and your `ESpeakNode` nodes.
    *   When the user enters text and presses the "speak" button:
        *   It calls `espeak_node.synthesize(text)` to get the raw audio data.
        *   It creates a Godot `AudioStreamWAV`.
        *   **It explicitly configures the `AudioStreamWAV` format** (16-bit, 22050 Hz, mono) so Godot knows how to correctly interpret the raw audio data received from the GDExtension.
        *   It assigns the audio data to the `AudioStreamWAV` and plays it via the `AudioStreamPlayer`.
    *   When the user interacts with the `CheckButton` (`SpanishToggle`):
        *   It calls `espeak_node.set_language()` with the appropriate language code ("es" or "en") to change the eSpeak-NG voice.

In summary, the GDExtension acts as a bridge between Godot and the `espeak-ng` library, handling speech synthesis and delivering audio data in a format Godot can play directly in memory.