#ifndef ESPEAK_NODE_H
#define ESPEAK_NODE_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/classes/os.hpp> // Added this line
#include <godot_cpp/variant/utility_functions.hpp> // For print

namespace godot {

class ESpeakNode : public Node {
    GDCLASS(ESpeakNode, Node)

protected:
    static void _bind_methods();

public:
    ESpeakNode();
    ~ESpeakNode();

    PackedByteArray synthesize(const String &text);
    void set_language(const String &p_lang_code);
};

}

#endif // ESPEAK_NODE_H