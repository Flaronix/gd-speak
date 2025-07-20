
#include "espeak_node.h"
#include <espeak-ng/speak_lib.h>
#include <godot_cpp/core/memory.hpp>

using namespace godot;

static int callback_call_count = 0;

// This function is a "callback" that espeak-ng uses to give us the audio data.
int synth_callback(short *wav, int num_samples, espeak_EVENT *events) {
    callback_call_count++;
    if (wav == nullptr || num_samples == 0) {
        UtilityFunctions::print("synth_callback: No audio data or 0 samples. num_samples: " + String::num_int64(num_samples));
        return 0; // Continue
    }

    UtilityFunctions::print("synth_callback: Received " + String::num_int64(num_samples) + " samples. Call count: " + String::num_int64(callback_call_count));

    // The user_data pointer holds our byte array.
    PackedByteArray *byte_array = (PackedByteArray *)events->user_data;
    if (byte_array) {
        int current_size = byte_array->size();
        byte_array->resize(current_size + num_samples * sizeof(short));
        memcpy(byte_array->ptrw() + current_size, wav, num_samples * sizeof(short));
        UtilityFunctions::print("synth_callback: Total WAV data size: " + String::num_int64(byte_array->size()));
    } else {
        UtilityFunctions::print("synth_callback: byte_array is null!");
    }

    return 0; // 0 to continue, 1 to stop.
}

void ESpeakNode::_bind_methods() {
    ClassDB::bind_method(D_METHOD("synthesize", "text"), &ESpeakNode::synthesize);
    ClassDB::bind_method(D_METHOD("set_language", "lang_code"), &ESpeakNode::set_language);
}

ESpeakNode::ESpeakNode() {
    // Initialize espeak. The first parameter is the output type.
    // AUDIO_OUTPUT_SYNCHRONOUS gives us the data via a callback.
    // The second parameter is the buffer length, 0 for default.
    // The third is the path for espeak-ng-data, NULL for default.
    // The fourth is options, 0 for default.
    String espeak_data_path = OS::get_singleton()->get_executable_path().get_base_dir().path_join("bin/espeak-ng-data"); // Relative path to project bin/espeak-ng-data
    UtilityFunctions::print("ESpeak data path: " + espeak_data_path);
    int espeak_init_result = espeak_Initialize(AUDIO_OUTPUT_SYNCHRONOUS, 0, espeak_data_path.utf8().get_data(), 0);
    UtilityFunctions::print("espeak_Initialize result: " + String::num_int64(espeak_init_result));
    espeak_SetSynthCallback(synth_callback);

    // Set default language to English (en) on initialization
    espeak_SetVoiceByName("en");
    UtilityFunctions::print("Default language set to: en");
}

ESpeakNode::~ESpeakNode() {
    espeak_Terminate();
}

void ESpeakNode::set_language(const String &p_lang_code) {
    UtilityFunctions::print("C++: set_language llamado con: " + p_lang_code);
    CharString lang_code_utf8 = p_lang_code.utf8();
    int result = espeak_SetVoiceByName(lang_code_utf8.get_data());
    if (result == EE_OK) {
        UtilityFunctions::print("C++: Idioma establecido a: " + p_lang_code);
    } else {
        UtilityFunctions::print("C++: Error al establecer idioma a " + p_lang_code + ": " + String::num_int64(result));
    }
}

PackedByteArray ESpeakNode::synthesize(const String &text) {
    PackedByteArray raw_audio_data;

    // The user data is now passed directly to the synth function.
    void* user_data_ptr = &raw_audio_data;

    // Convert Godot String to a C-style string for espeak.
    CharString utf8_text = text.utf8();
    const char *c_text = utf8_text.get_data();

    // Synthesize! This will call our `synth_callback` and populate raw_audio_data.
    // The last parameter is the user_data pointer.
    int synth_result = espeak_Synth(c_text, strlen(c_text) + 1, 0, POS_CHARACTER, 0, espeakCHARS_UTF8, NULL, user_data_ptr);
    UtilityFunctions::print("espeak_Synth result: " + String::num_int64(synth_result));

    // Wait for all queued speech to be synthesized.
    espeak_Synchronize();
    UtilityFunctions::print("espeak_Synchronize() called.");
    UtilityFunctions::print("Callback call count: " + String::num_int64(callback_call_count));
    UtilityFunctions::print("Raw audio data size after synth and synchronize: " + String::num_int64(raw_audio_data.size()));

    // Return only the raw audio data. Godot's AudioStreamWAV will add the header.
    return raw_audio_data;
}
