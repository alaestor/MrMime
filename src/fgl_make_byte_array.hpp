#pragma once
#ifndef FGL_MAKE_BYTE_ARRAY_HPP_INCLUDED
#define FGL_MAKE_BYTE_ARRAY_HPP_INCLUDED

#include <cstddef> // size_t, byte
#include <utility> // index_sequence, make_index_sequence
#include <array>
#include <type_traits>
#include <concepts>

namespace fgl {

template <class T>
concept StdArrayOfBytes = requires
{
	requires std::same_as<
		std::byte,
		std::remove_cv_t<typename T::value_type>>;

	requires std::same_as<
		std::array<typename T::value_type, sizeof(T)>,
		std::remove_cv_t<T>>;
};

template <class T>
concept StdArrayOfConstBytes = requires
{
	requires StdArrayOfBytes<T>;
	requires std::is_const_v<typename T::value_type>;
};

/// returns a std::array<std::byte> from cstring literal. removes null term.
template<std::size_t LEN> [[nodiscard]]
constexpr std::array<const std::byte, LEN-1> make_byte_array(
	const char (&cstr)[LEN])
{
	return
		[&]<std::size_t ... I>(std::index_sequence<I...>)
		constexpr -> std::array<const std::byte, LEN-1>
		{
			return {{static_cast<const std::byte>(cstr[I])...}};
		}(std::make_index_sequence<LEN-1>());
}

} // namespace fgl

#endif // FGL_MAKE_BYTE_ARRAY_HPP_INCLUDED
