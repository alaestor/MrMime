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

	template<std::size_t LEN> [[nodiscard]]
	constexpr auto operator<<(const char (&cstr)[LEN]) const
	{
		return Byte_Signature_Factory(
			std::tuple_cat(m_sig, std::tuple(fgl::make_byte_array(cstr)))
		);
	}

	[[nodiscard]] constexpr auto operator<<(const SkipBytes sb) const
	{ return Byte_Signature_Factory(std::tuple_cat(m_sig, std::tuple(sb))); }

	template<fgl::StdArrayOfBytes ARRAY_T> [[nodiscard]]
	bool compare(FileType& ft_out, const ARRAY_T& arr) const
	{
		const bool arr_matches_sig{
			std::apply(
				[&]<typename ... ELEMENTS>(const ELEMENTS& ... elements)
				-> bool
				{
					auto cursor{ arr.cbegin() };
					const auto cend{ arr.cend() };
					return (compare_element(cursor, cend, elements) && ...);
				},
				m_sig
			)
		};

		if (arr_matches_sig)
			ft_out = m_id;

		return arr_matches_sig;
	}

private:

	template<typename TUPLE_T_but_different> [[nodiscard]]
	constexpr auto Byte_Signature_Factory(const TUPLE_T_but_different t) const
	{ return Byte_Signature<TUPLE_T_but_different>(m_id, t); } // AAAAAAAAAAAA

	// signature tuple-element specializations: SkipBytes and StdArrayOfBytes

	[[nodiscard]]
	static constexpr std::size_t size_of_element(const SkipBytes sb)
	{ return sb; }

	template <fgl::StdArrayOfBytes AOB_T> [[nodiscard]]
	static constexpr std::size_t size_of_element(const AOB_T&)
	{ return sizeof(AOB_T); }

	template<std::forward_iterator ITER_T>
	[[nodiscard]]
	static bool compare_element(
		ITER_T& cursor,
		const ITER_T cend,
		const SkipBytes element)
	{
		const auto range_end{ cursor + element.bytes_to_skip() };
		assert(range_end <= cend);
		cursor = range_end; // advance cursor
		return true;
	}

	template<std::forward_iterator ITER_T, fgl::StdArrayOfBytes SIG_AOB_T>
	[[nodiscard]] static bool compare_element(
		ITER_T& cursor,
		const ITER_T cend,
		const SIG_AOB_T& element)
	{
		const auto cmp_range_begin{ cursor };
		const auto cmp_range_end{ cursor + element.size() };
		assert(cmp_range_end <= cend);

		const bool eq_result{
			std::equal(
				cmp_range_begin, cmp_range_end,
				element.cbegin(), element.cend())
		};

		cursor = cmp_range_end; // advance cursor

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
