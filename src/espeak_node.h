#ifndef ESPEAK_NODE_H
#define ESPEAK_NODE_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
// #include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp> // Using for contextual relative file path in `espeak_node.cpp` -> `ESpeaker()`
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/core/class_db.hpp>


namespace godot {

class ESpeaker : public Node {
    GDCLASS(ESpeaker, Node)

private:
    bool debug_print = false;

protected:
    static void _bind_methods();

public:
    ESpeaker();
    ~ESpeaker();

    void set_debug_print(bool p_enabled);
    bool get_debug_print() const;

    PackedByteArray synthesize(const String &text);
    void set_language(const String &p_lang_code);
};

}

#endif // ESPEAK_NODE_H