#pragma once
#ifndef FGL_ZIP_HPP_INCLUDED
#define FGL_ZIP_HPP_INCLUDED

#include <cstddef> // size_t, ptrdiff_t
#include <ranges> // forward_range
#include <functional> // ref
#include <iterator> // forward_iterator, forward_iterator_tag, iterator_traits
#include <tuple>

namespace fgl {
namespace zip {

/// Forward Zip Iterator

template <std::forward_iterator ... Iters_t>
class ZipIterator
{
	template <std::forward_iterator Iter>
	using GetIterReferenceType = typename std::iterator_traits<Iter>::reference;

	static constexpr inline auto accessType(auto& v)
	{ return std::ref(v); }

	static constexpr inline auto accessType(auto&& v)
	{ return v; }

	std::tuple<Iters_t...> m_iters;

public:
	using value_type = std::tuple<GetIterReferenceType<Iters_t>...>;
	using difference_type = std::ptrdiff_t;
	using iterator_category = std::forward_iterator_tag;

	explicit constexpr ZipIterator(Iters_t&& ... args)
	: m_iters(std::forward<Iters_t>(args)...)
	{}

	constexpr ZipIterator& operator++()
	{
		std::apply([](auto && ... args) noexcept { ((++args),...); }, m_iters);
		return *this;
	}

	constexpr ZipIterator operator++(int)
	{
		ZipIterator tmp = *this;
		++*this;
		return tmp;
	}

	// sigh... so much for "self documenting" code
	constexpr bool operator==(const ZipIterator& rhs) const
	{ // true if any (lhs iterator == rhs iterator) to work with uneven length!
		return [&]<std::size_t ... i>(std::index_sequence<i...>) -> bool
		{ // overly complicated way of doing ((lhs[i] == rhs[i]) || ...)
			return ((std::get<i>(m_iters)==std::get<i>(rhs.m_iters)) || ...);
		}(std::index_sequence_for<Iters_t...>());
	}

	constexpr bool operator!=(const ZipIterator& rhs) const
	{ return !(*this == rhs); }

	constexpr value_type operator*() const
	{
		return [&]<std::size_t ... i>(std::index_sequence<i...>) -> value_type
		{
			return std::tuple(accessType(*std::get<i>(m_iters))...);
		}(std::index_sequence_for<Iters_t...>());
	}
};

static_assert(std::forward_iterator<ZipIterator<>>);

/// Ranged Helpers

template <std::forward_iterator ... Iters_t>
class Zipped
{
	using ZipIter_t = ZipIterator<Iters_t...>;

	ZipIter_t m_alpha;
	const ZipIter_t m_omega;

public:
	constexpr Zipped() = delete;

	constexpr explicit Zipped(ZipIter_t&& begin, ZipIter_t&& end):
		m_alpha(std::forward<ZipIter_t>(begin)),
		m_omega(std::forward<ZipIter_t>(end))
	{}

	constexpr explicit Zipped(ZipIter_t& begin, const ZipIter_t& end):
		m_alpha(begin),
		m_omega(end)
	{}

	constexpr auto begin() const noexcept
	{ return m_alpha; }

	constexpr auto end() const noexcept
	{ return m_omega; }
};

static_assert(std::ranges::forward_range<Zipped<>>);

/// Helper factory functions
/// NOTE: use czip() for const iterators!

constexpr auto zip(std::ranges::forward_range auto& ... args)
{
	return Zipped(
		ZipIterator(std::begin(args)...),
		ZipIterator(std::end(args)...)
	);
};

constexpr auto czip(const std::ranges::forward_range auto& ... args)
{
	return Zipped(
		ZipIterator(std::cbegin(args)...),
		ZipIterator(std::cend(args)...)
	);
};

}// namespace zip
}// namespace fgl

#endif // FGL_ZIP_HPP_INCLUDED
