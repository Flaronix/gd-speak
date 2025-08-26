
#include "espeak_node.h"
#include <espeak-ng/speak_lib.h>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

static int callback_call_count = 0;
bool callback_debug_print = false; // Only used by synth_callback. Hard-coded since synth_callback is static and idk the best way to do this in C++ lol


void ESpeaker::set_debug_print(bool p_enabled) {
    debug_print = p_enabled;
    UtilityFunctions::print("[ESpeak C++] debug_print set to: " + String(p_enabled ? "true" : "false"));
}

bool ESpeaker::get_debug_print() const {
    return debug_print;
}

// This function is a "callback" that espeak-ng uses to give us the audio data.
int synth_callback(short *wav, int num_samples, espeak_EVENT *events) {
    callback_call_count++;
    if (wav == nullptr || num_samples == 0) {
        if (callback_debug_print) UtilityFunctions::print("[ESpeak C++] synth_callback: No audio data or 0 samples. num_samples: " + String::num_int64(num_samples));
        return 0; // Continue
    }

    if (callback_debug_print) UtilityFunctions::print("[ESpeak C++] synth_callback: Received " + String::num_int64(num_samples) + " samples. Call count: " + String::num_int64(callback_call_count));

    // The user_data pointer holds our byte array.
    PackedByteArray *byte_array = (PackedByteArray *)events->user_data;
    if (byte_array) {
        int current_size = byte_array->size();
        byte_array->resize(current_size + num_samples * sizeof(short));
        memcpy(byte_array->ptrw() + current_size, wav, num_samples * sizeof(short));
        if (callback_debug_print) UtilityFunctions::print("[ESpeak C++] synth_callback: Total WAV data size: " + String::num_int64(byte_array->size()));
    } else {
        if (callback_debug_print) UtilityFunctions::print("[ESpeak C++] synth_callback: byte_array is null!");
    }

    return 0; // 0 continues, 1 stops
}

void ESpeaker::_bind_methods() {
    ClassDB::bind_method(D_METHOD("synthesize", "text"), &ESpeaker::synthesize);
    ClassDB::bind_method(D_METHOD("set_language", "lang_code"), &ESpeaker::set_language);

    ClassDB::bind_method(D_METHOD("set_debug_print", "enabled"), &ESpeaker::set_debug_print);
    ClassDB::bind_method(D_METHOD("get_debug_print"), &ESpeaker::get_debug_print);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_print"), "set_debug_print", "get_debug_print");
}

ESpeaker::ESpeaker() {
    String espeak_data_path = ProjectSettings::get_singleton()->globalize_path("res://addons/gd-speak/espeak-ng-data");
    if (debug_print) UtilityFunctions::print("[ESpeak C++] ESpeak data path: " + espeak_data_path);

    // espeak_Initialize(output type, buffer length (0 default), path to espeak-ng-data, options (0 default))
    espeak_Initialize(AUDIO_OUTPUT_SYNCHRONOUS, 0, espeak_data_path.utf8().get_data(), 0);
    espeak_SetSynthCallback(synth_callback);

    espeak_SetVoiceByName("en"); // Set English (en) as default language on init
    if (debug_print) UtilityFunctions::print("[ESpeak C++] Default language set to: en");
}

ESpeaker::~ESpeaker() {
    espeak_Terminate();
}

void ESpeaker::set_language(const String &p_lang_code) {
    if (debug_print) UtilityFunctions::print("[ESpeak C++] set_language called with language code: " + p_lang_code);
    CharString lang_code_utf8 = p_lang_code.utf8();
    int result = espeak_SetVoiceByName(lang_code_utf8.get_data());
    if (result == EE_OK) {
        if (debug_print) UtilityFunctions::print("[ESpeak C++] Language set to: " + p_lang_code);
    } else {
        UtilityFunctions::print("[ESpeak C++] Error setting language to " + p_lang_code + ": " + String::num_int64(result));
    }
}

PackedByteArray ESpeaker::synthesize(const String &text) {
    PackedByteArray raw_audio_data;

    // The user data is now passed directly to the synth function.
    void* user_data_ptr = &raw_audio_data;

    // Convert Godot String to a C-style string for espeak.
    CharString utf8_text = text.utf8();
    const char *c_text = utf8_text.get_data();

    // Synthesize! This will call our `synth_callback` and populate raw_audio_data.
    // The last parameter is the user_data pointer.
    int synth_result = espeak_Synth(c_text, strlen(c_text) + 1, 0, POS_CHARACTER, 0, espeakCHARS_UTF8, NULL, user_data_ptr);
    if (debug_print) UtilityFunctions::print("[ESpeak C++] espeak_Synth result: " + String::num_int64(synth_result));

    // Wait for all queued speech to be synthesized.
    espeak_Synchronize();
    if (debug_print) UtilityFunctions::print("[ESpeak C++] espeak_Synchronize() called.");
    if (debug_print) UtilityFunctions::print("[ESpeak C++] Callback call count: " + String::num_int64(callback_call_count));
    if (debug_print) UtilityFunctions::print("[ESpeak C++] Raw audio data size after synth and synchronize: " + String::num_int64(raw_audio_data.size()));

    // Return only the raw audio data. Godot's AudioStreamWAV will add the header.
    return raw_audio_data;
}
