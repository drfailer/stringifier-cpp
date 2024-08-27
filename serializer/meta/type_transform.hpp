#ifndef SERIALIZER_TYPE_TRANSFORM_H
#define SERIALIZER_TYPE_TRANSFORM_H
#include "concepts.hpp"
#include <array>
#include <cstddef>
#include <type_traits>

/// @brief serializer meta-functions namespace
namespace serializer::mtf {

/// @brief Allow easy access to the type of the elements_ in a members_.
///        We don't use value_type directly in case of a not well implemented
///        members_. Though, we assume that the iterator respects the standard.
template <typename T> struct iter_value {
    using type = typename base_t<T>::iterator::value_type;
};

/// @brief Specific case for std::array (it is iterable using pointers and not
///        proper iterators).
template <typename T, size_t N> struct iter_value<std::array<T, N>> {
    using type = T;
};

/// @brief iter_value shorthand.
template <typename T> using iter_value_t = iter_value<T>::type;

/// @brief Check if a type T is in the parameter pac Types. value is true if T
///        is in Types.
template <typename T, typename... Types> struct contains;
template <typename T> struct contains<T> {
    static constexpr bool value = false;
};
template <typename T, typename H, typename... Tail>
struct contains<T, H, Tail...> {
    static constexpr bool value =
        std::is_same_v<T, H> || contains<T, Tail...>::value;
};

/// @brief Shorthand for contains.
template <typename T, typename... Types>
constexpr bool contains_v = contains<base_t<T>, Types...>::value;

/// @brief Put the type H at the beginning of the tuple T
template <typename H, typename T> struct tuple_push_front;

template <typename H, template <typename...> class T, typename... Types>
struct tuple_push_front<H, T<Types...>> {
    using type = T<H, Types...>;
};

template <typename H, typename T>
using tuple_push_front_t = typename tuple_push_front<H, T>::type;

/// @brief Remove const specifier on the types contained in the tuple T.
template <typename T> struct remove_const_tuple;

template <template <typename...> class T> struct remove_const_tuple<T<>> {
    using type = T<>;
};

template <template <typename...> class T, typename H, typename... Types>
struct remove_const_tuple<T<H, Types...>> {
    using type =
        tuple_push_front_t<std::remove_const_t<H>,
                           typename remove_const_tuple<T<Types...>>::type>;
};

template <typename T1, typename T2>
struct remove_const_tuple<std::pair<T1, T2>> {
    using type = std::pair<std::remove_const_t<T1>, std::remove_const_t<T2>>;
};

template <typename T>
using remove_const_tuple_t = typename remove_const_tuple<T>::type;

/// @brief Remove const on all types
/// TODO: move this elsewhere
template <typename T> struct remove_const {
    using type = std::remove_const_t<T>;
};

template <typename T> using remove_const_t = typename remove_const<T>::type;

template <concepts::TupleLike T> struct remove_const<T> {
    using type = mtf::remove_const_tuple_t<T>;
};

} // end namespace serializer::mtf

#endif