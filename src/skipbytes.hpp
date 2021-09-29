#pragma once
#ifndef MRMIME_SKIPBYTES_HPP_INCLUDED
#define MRMIME_SKIPBYTES_HPP_INCLUDED

#include <cstddef> // std::size_t
#include <cstdint> // uint8_t

namespace MrMime {
namespace internal {

class SkipBytes
{
	using SizeType = uint8_t;
	const SizeType m_bytes{ 0 };

	public:
	constexpr SkipBytes(const SizeType number_of_bytes_to_skip)
	: m_bytes(number_of_bytes_to_skip)
	{}

	constexpr operator SizeType() const
	{ return m_bytes; }

	constexpr std::size_t bytes_to_skip() const
	{ return static_cast<std::size_t>(m_bytes); }
};

} // namespace internal
} // namespace MrMime

#endif // MRMIME_SKIPBYTES_HPP_INCLUDED
