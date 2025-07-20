# Documentación de la Integración eSpeak-NG en Godot

Este documento proporciona pautas para trabajar con la GDExtension de eSpeak-NG en tu proyecto Godot.

## 1. Cómo Actualizar Cambios en eSpeak-NG y Recompilar la GDExtension

Si realizas modificaciones en el código fuente de eSpeak-NG (dentro de `gextension_espeak/espeak-ng-src`) o en el código de la GDExtension (`gextension_espeak/src`), necesitarás recompilar la GDExtension para que los cambios se reflejen en tu proyecto Godot.

Sigue estos pasos:

1.  **Navega al directorio de la GDExtension:**
    ```bash
    cd /home/soykhaler/espeakgodot/gextension_espeak
    ```

2.  **Recompila la GDExtension usando SCons:**
    ```bash
    scons
    ```
    Este comando reconstruirá la librería `libespeakgodotextension.so` ubicada en `espeakgodot/bin/`.

3.  **Reinicia el editor de Godot:** Después de la recompilación, es crucial cerrar y volver a abrir el editor de Godot para que cargue la nueva versión de la librería. Si solo ejecutas la escena, Godot podría seguir usando la versión anterior en memoria.

## 2. ¿eSpeak-NG Necesita Estar Instalado en el Sistema?

**No, eSpeak-NG no necesita estar instalado en tu sistema operativo.**

La GDExtension que has compilado (`libespeakgodotextension.so`) incluye la librería `libespeak-ng.so` y sus archivos de datos (`espeak-ng-data`) de forma embebida o local dentro de tu proyecto Godot (específicamente en la carpeta `espeakgodot/bin/`).

Esto significa que tu proyecto es **autocontenido** en lo que respecta a eSpeak-NG. Puedes distribuir tu proyecto Godot y eSpeak-NG funcionará sin requerir que el usuario final tenga eSpeak-NG preinstalado en su sistema.

## 3. Cómo Funciona el Código (Alto Nivel)

La integración de eSpeak-NG en tu proyecto Godot se logra a través de una **GDExtension** escrita en C++. Aquí un resumen de cómo interactúan las partes:

*   **`espeak.gdextension` (Archivo de Configuración de Godot):**
    *   Este archivo le dice a Godot que existe una GDExtension y dónde encontrar la librería compilada (`libespeakgodotextension.so`).

*   **`libespeakgodotextension.so` (Librería Compilada - C++):**
    *   Es el corazón de la integración. Contiene el código C++ que se comunica directamente con la librería `espeak-ng`.
    *   Define una clase `ESpeakNode` que hereda de `Node` de Godot, haciéndola accesible en tus escenas de Godot.

*   **`ESpeakNode` (Clase C++):**
    *   **Inicialización (`ESpeakNode::ESpeakNode()`):** Cuando se crea una instancia de `ESpeakNode` en Godot, su constructor inicializa la librería `espeak-ng`. Le indica a `espeak-ng` dónde encontrar sus archivos de datos (`espeak-ng-data`) y configura un "callback" para recibir los datos de audio.
    *   **Síntesis de Voz (`ESpeakNode::synthesize(text)`):** Este método toma un `String` de Godot (el texto a hablar), lo convierte a un formato que `espeak-ng` entiende y le pide a `espeak-ng` que sintetice el audio.
        *   `espeak-ng` no devuelve el audio inmediatamente; en su lugar, llama repetidamente a una función de "callback" (`synth_callback`) a medida que genera fragmentos de audio.
        *   El método `synthesize` espera a que `espeak-ng` termine de generar todo el audio (`espeak_Synchronize()`) y recolecta todos los fragmentos de audio en un `PackedByteArray`.
        *   **Importante:** Este método devuelve *solo los datos de audio sin procesar* (PCM de 16 bits, mono, 22050 Hz), sin el encabezado WAV.
    *   **Cambio de Idioma (`ESpeakNode::set_language(lang_code)`):** Este método toma un código de idioma (ej. "en", "es") y le indica a `espeak-ng` que cambie la voz utilizada para la síntesis.

*   **`main.gd` (Script GDScript):**
    *   Este script es el controlador de tu interfaz de usuario en Godot.
    *   Obtiene referencias a los nodos `LineEdit`, `AudioStreamPlayer` y tu `ESpeakNode`.
    *   Cuando el usuario introduce texto y presiona el botón "hablar":
        *   Llama a `espeak_node.synthesize(text)` para obtener los datos de audio sin procesar.
        *   Crea un `AudioStreamWAV` en Godot.
        *   **Configura explícitamente el formato del `AudioStreamWAV`** (16 bits, 22050 Hz, mono) para que Godot sepa cómo interpretar correctamente los datos de audio sin procesar que recibe de la GDExtension.
        *   Asigna los datos de audio al `AudioStreamWAV` y lo reproduce a través del `AudioStreamPlayer`.
    *   Cuando el usuario interactúa con el `CheckButton` (`SpanishToggle`):
        *   Llama a `espeak_node.set_language()` con el código de idioma apropiado ("es" o "en") para cambiar la voz de eSpeak-NG.

En resumen, la GDExtension actúa como un puente entre Godot y la librería `espeak-ng`, manejando la síntesis de voz y la entrega de los datos de audio en un formato que Godot puede reproducir directamente en memoria.
