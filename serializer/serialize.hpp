#ifndef SERIALIZER_SERIALIZE_H
#define SERIALIZER_SERIALIZE_H
#include "meta/concepts.hpp"
#include "serializer/serializer.hpp"
#include "tools/bytes.hpp"
#include "tools/context.hpp"
#include "tools/super.hpp"
#include "tools/type_table.hpp"

// TODO: use always inline attribute

/// @brief serializer namespace
namespace serializer {

/// @brief Serialize the arguments into the memory buffer at the given position.
/// @tparam Ser Serializer type.
/// @param mem  Buffer in which the serialized data will be stored.
/// @param pos  Start position in the buffer for serializing the data.
/// @param args Values to serialize
/// @return Position of the next element in the buffer.
template <typename Ser>
inline constexpr size_t serialize(auto &mem, size_t pos, auto &&...args) {
    using mem_t = decltype(mem);
    [[maybe_unused]] bool first_level = pos == 0;

    Ser serializer(mem, pos);
    (
        [&serializer, &args] {
            if constexpr (SerializerFunction(args, serializer)) {
                args(tools::Context<tools::Phases::Serialization,
                                    decltype(serializer)>(serializer));
            } else {
                serializer.serialize_(args);
            }
        }(),
        ...);
    if constexpr (!mtf::is_serializer_bytes_v<mem_t> &&
                  concepts::Resizeable<mem_t>) {
        if (first_level) [[unlikely]] {
            mem.resize(serializer.pos);
        }
    }
    return serializer.pos;
}

/// @brief Deserialize the arguments from the memory buffer at the given
///        position.
/// @tparam Ser Serializer type.
/// @param mem  Buffer in which the serialized data is be stored.
/// @param pos  Start position in the buffer for deserializing the data.
/// @param args references to the variables that are deserialized.
/// @return Position of the next element in the buffer.
template <typename Ser>
inline constexpr size_t deserialize(auto &mem, size_t pos, auto &&...args) {
    Ser serializer(mem, pos);
    (
        [&serializer, &args] {
            if constexpr (SerializerFunction(args, serializer)) {
                args(tools::Context<tools::Phases::Deserialization,
                                    decltype(serializer)>(serializer));
            } else {
                serializer.deserialize_(args);
            }
        }(),
        ...);
    return serializer.pos;
}

/// @brief Serialize obj into mem at pos using a bit_cast.
/// @tparam T Object type (this).
/// @param mem Buffer in which the serialized data is be stored.
/// @param pos Start position in the buffer for serializing the data.
/// @param obj Pointer to the object that is serialized.
/// @return Position of the next element in the buffer.
template <typename T>
    requires(std::is_trivially_copyable_v<T> &&
             std::is_trivially_copy_assignable_v<T>)
inline constexpr size_t serializeStruct(auto &mem, size_t pos, T const *obj) {
    constexpr size_t nb_bytes = sizeof(*obj);
    Serializer<decltype(mem)> serializer(mem, pos);
    using byte_type = std::remove_cvref_t<decltype(mem[0])>;
    serializer.append(std::bit_cast<const byte_type *>(obj), nb_bytes);
    return serializer.pos;
}

/// @brief Deserialize obj from mem at pos using a bit_cast.
/// @tparam T Object type (this).
/// @param mem Buffer in which the serialized data is be stored.
/// @param pos Start position in the buffer for deserializing the data.
/// @param obj Pointer to the object that is deserialized.
/// @return Position of the next element in the buffer.
template <typename T>
inline constexpr size_t deserializeStruct(auto &mem, size_t pos, T *obj) {
    *obj = *std::bit_cast<const decltype(obj)>(mem.data() + pos);
    return pos + sizeof(*obj);
}

/// @brief Create a function for serializing the given object using the
///        accessors.
/// @param obj       Object to serialize.
/// @param accessors Accessors to the attributes (or the attributes themselves).
/// @return Function (void(auto bytes, size_t pos = 0)) that serialize the
///         attributes accessible via `accessors`
template <typename Ser>
constexpr inline auto bindSerialize(auto &obj, auto &&...accessors) {
    return [=, &obj](auto &mem, size_t pos = 0) {
        return serialize<Ser>(mem, pos, std::invoke(accessors, obj)...);
    };
}

/// @brief Create a function for deserializing the given object using the
///        accessors.
/// @param obj       Object to deserialize.
/// @param accessors Accessors to the attributes (or the attributes themselves).
/// @return Function (void(auto bytes, size_t pos = 0)) that deserialize the
///         attributes accessible via `accessors`
template <typename Ser>
constexpr inline auto bindDeserialize(auto &obj, auto &&...accessors) {
    return [=, &obj](auto &mem, size_t pos = 0) {
        return tools::deserializerAccessors<Ser>(mem, pos, obj, accessors...);
    };
}

// TODO: change this
using default_mem_type = tools::Bytes<uint8_t>;
/* using default_mem_type = std::vector<uint8_t>; */

} // end namespace serializer

#endif
