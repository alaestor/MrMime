#pragma once
#ifndef MRMIME_BYTE_SIGNATURE_HPP_INCLUDED
#define MRMIME_BYTE_SIGNATURE_HPP_INCLUDED

#include <cstddef> // std::size_t
#include <tuple>
#include <cassert>

#include "filetype_enum.h"
#include "skipbytes.hpp"
#include "fgl_make_byte_array.hpp"

namespace MrMime {
namespace internal {

/*** TODO
Tuple concepts: expects tuple<array<byte>>,

To accomodate KJ:
break out comparator methods to be loose functions in own hpp,
ideally taking a range view or span(?), rather than array<byte>& */

template<typename TUPLE_T> // expects tuple<array<byte>>, TODO concept
struct Byte_Signature
{
	const FileType m_id;
	const TUPLE_T m_sig;

	explicit constexpr Byte_Signature(const FileType ft, const TUPLE_T sig)
	: m_id(ft), m_sig(sig)
	{}

	[[nodiscard]] constexpr std::size_t size() const
	{
		return std::apply(
			[]<typename ... ELEMENT_TS>(const ELEMENT_TS& ... elements)
			constexpr -> std::size_t
			{
				return (size_of_element(elements) + ...);
			},
			m_sig
		);
	}

	/// Doesn't perform bounds-checking; onus is on caller!
	// Undefined behavior if ARRAY_T is less than size()
	template<fgl::StdArrayOfBytes ARRAY_T> [[nodiscard]]
	bool compare(FileType& ft_out, const ARRAY_T& arr) const
	{
		// this could be done with more info about TUPLE_T's elements
		// static_assert(sizeof(ARRAY_T) >= size());
		const bool arr_matches_sig{
			std::apply(
				[&]<typename ... ELEMENTS>(const ELEMENTS& ... elements)
				-> bool
				{
					auto cursor{ arr.cbegin() };
					return (compare_element(cursor, elements) && ...);
				},
				m_sig
			)
		};

		if (arr_matches_sig)
			ft_out = m_id;

		return arr_matches_sig;
	}

	template<std::size_t LEN> [[nodiscard]]
	constexpr auto operator<<(const char (&cstr)[LEN]) const
	{
		return Byte_Signature_Factory(
			std::tuple_cat(m_sig, std::tuple(fgl::make_byte_array(cstr))));
	}

	[[nodiscard]] constexpr auto operator<<(const SkipBytes sb) const
	{ return Byte_Signature_Factory(std::tuple_cat(m_sig, std::tuple(sb))); }

private:

	template<typename TUPLE_T_but_different> [[nodiscard]] // TODO tup concept
	constexpr auto Byte_Signature_Factory(const TUPLE_T_but_different t) const
	{ return Byte_Signature<TUPLE_T_but_different>(m_id, t); }

	// signature tuple-element specializations: SkipBytes and StdArrayOfBytes

	[[nodiscard]]
	static constexpr std::size_t size_of_element(const SkipBytes sb)
	{ return sb; }

	template <fgl::StdArrayOfBytes AOB_T> [[nodiscard]]
	static constexpr std::size_t size_of_element(const AOB_T&)
	{ return sizeof(AOB_T); }

	/// Doesn't perform bounds-checking; onus is on caller!
	template<std::forward_iterator ITER_T>
	requires std::same_as<std::byte, std::iter_value_t<ITER_T>>
	[[nodiscard]] constexpr static bool compare_element(
		ITER_T& cursor, const SkipBytes element)
	{
		cursor += element.bytes_to_skip(); // advance cursor
		return true;
	}

	/// Doesn't perform bounds-checking; onus is on caller!
	template<std::forward_iterator ITER_T, fgl::StdArrayOfBytes SIG_AOB_T>
	requires std::same_as<std::byte, std::iter_value_t<ITER_T>>
	[[nodiscard]] static bool compare_element(
		ITER_T& cursor, const SIG_AOB_T& element)
	{
		const auto cursor_end{ cursor + element.size() };

		const bool eq_result{
			std::equal(cursor, cursor_end, element.cbegin(), element.cend())
		};

		cursor = cursor_end; // advance cursor

		return eq_result;
	}
};

struct Byte_Signature_Stream_Starter
{
	const FileType m_id;

	explicit constexpr Byte_Signature_Stream_Starter(const FileType ft)
	: m_id(ft)
	{}

	template<std::size_t LEN>
	[[nodiscard]] constexpr auto operator<<(const char (&cstr)[LEN]) const
	{ return Byte_Signature(m_id, std::tuple(fgl::make_byte_array(cstr))); }

	[[nodiscard]] constexpr auto operator<<(const SkipBytes sb) const
	{ return Byte_Signature(m_id, std::tuple(sb)); }
};

} // namespace internal
} // namespace MrMime

#endif // MRMIME_BYTE_SIGNATURE_HPP_INCLUDED
