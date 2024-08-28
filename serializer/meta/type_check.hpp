#ifndef SERIALIZER_TYPE_CHECK_H
#define SERIALIZER_TYPE_CHECK_H
#include "../tools/bytes.hpp"
#include <memory>
#include <string>
#include <type_traits>

/// @brief serializer meta-functions namespace
namespace serializer::mtf {

/// @brief Removes the reference and the const on a type.
template <typename T>
using base_t = typename std::remove_const_t<std::remove_reference_t<T>>;

/// @brief Check if a pointer is concrete or not.
template <typename T>
constexpr bool is_concrete_ptr_v =
    std::is_pointer_v<base_t<T>> &&
    !std::is_polymorphic_v<std::remove_pointer_t<base_t<T>>> &&
    !std::is_abstract_v<std::remove_pointer_t<base_t<T>>>;

/// @brief Check if a type T is a string / const string.
template <typename T>
constexpr bool is_string_v = std::is_same_v<base_t<T>, std::string>;

/* shared pointers ************************************************************/

/// @brief Checks if a type SP is a shared_ptr
template <typename SP> struct is_shared : std::false_type {};

template <typename T> struct is_shared<std::shared_ptr<T>> : std::true_type {};

template <typename SP>
constexpr bool is_shared_v = is_shared<base_t<SP>>::value;

/* unique pointers ************************************************************/

/// @brief Checks if a type SP is a unique_ptr
template <typename SP> struct is_unique : std::false_type {};

template <typename T> struct is_unique<std::unique_ptr<T>> : std::true_type {};

template <typename SP>
constexpr bool is_unique_v = is_unique<base_t<SP>>::value;

/* smart pointers *************************************************************/

/// @brief Checks if a type is a shared or a unique_ptr
template <typename SP>
constexpr bool is_smart_ptr_v = is_shared_v<SP> || is_unique_v<SP>;

/// @brief Checks if the type T is an std::array.
template <typename T> struct is_std_array : std::false_type {};

template <typename T, size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

template <typename T>
constexpr bool is_std_array_v = is_std_array<base_t<T>>::value;

/// @brief True if T is a serializer Bytes
template <typename T> struct is_serializer_bytes : std::false_type {};

template <typename T>
struct is_serializer_bytes<serializer::tools::Bytes<T>> : std::true_type {};

/// @brief True if T is a serializer Bytes
template <typename T>
constexpr bool is_serializer_bytes_v = is_serializer_bytes<base_t<T>>::value;

/// @brief get the element type
template <typename T>
using element_type_t = typename mtf::base_t<T>::element_type;

/// @brief Helper constexpr for accessing default_constructible_v.
template <typename T>
constexpr bool is_default_constructible_v =
    std::is_default_constructible_v<std::remove_pointer_t<mtf::base_t<T>>>;

/* getter */

/// @brief True if the T is at setter
template <typename T> struct is_setter : std::false_type {};

template <typename R, class C, typename Arg>
struct is_setter<R (C::*)(Arg)> : std::true_type {};

template <typename T>
constexpr bool is_setter_v = is_setter<T>::value;

/// @brief Get type of the attribute corresponding to the setter
template <typename T> struct setter_arg_type;

template <typename R, class C, typename Arg>
struct setter_arg_type<R (C::*)(Arg)> {
    using type = Arg;
};

/// @brief Get type of the attribute corresponding to the setter
template <typename T>
using setter_arg_type_t = typename setter_arg_type<T>::type;

} // end namespace serializer::mtf

#endif
