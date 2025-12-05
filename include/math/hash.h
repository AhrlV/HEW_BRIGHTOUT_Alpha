
#ifndef HASH_H
#define HASH_H

#include <cstdint>
#include <string>

template<class T>
constexpr T fnv_prime;

template<class T>
constexpr T fnv_offset_basis;

template<>
constexpr std::uint32_t fnv_prime<std::uint32_t> = 16777619u;

template<>
constexpr std::uint64_t fnv_prime<std::uint64_t> = 1099511628211ull;

template<>
constexpr std::uint32_t fnv_offset_basis<std::uint32_t> = 2166136261u;

template<>
constexpr std::uint64_t fnv_offset_basis<std::uint64_t> = 14695981039346656037ull;


template<class T>
constexpr T fnv1a_hash(std::wstring str) noexcept
{
    T hash = fnv_offset_basis<T>;
    for (auto c : str)
    {
        hash ^= c;
        hash *= fnv_prime<T>;
    }

    return hash;
}

#endif 
