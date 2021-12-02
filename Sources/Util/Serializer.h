#pragma once

#include <functional>
#include <tuple>
#include <type_traits>
#include <cassert>

#include "IO/Stream.h"

template <class T, class... TArgs> decltype(void(T{ std::declval<TArgs>()... }), std::true_type{}) test_is_braces_constructible(int);
template <class, class...> std::false_type test_is_braces_constructible(...);
template <class T, class... TArgs> using is_braces_constructible = decltype(test_is_braces_constructible<T, TArgs...>(0));

struct any_type {
	template<class T>
	constexpr operator T(); // non explicit
};

template<class T>
auto to_tuple(T&& object) noexcept {
	using type = std::decay_t<T>;
	if constexpr (is_braces_constructible<type, any_type, any_type, any_type, any_type>{}) {
		auto&& [p1, p2, p3, p4] = object;
		return std::tie(p1, p2, p3, p4);
	}
	else if constexpr (is_braces_constructible<type, any_type, any_type, any_type>{}) {
		auto&& [p1, p2, p3] = object;
		return std::tie(p1, p2, p3);
	}
	else if constexpr (is_braces_constructible<type, any_type, any_type>{}) {
		auto&& [p1, p2] = object;
		return std::tie(p1, p2);
	}
	else if constexpr (is_braces_constructible<type, any_type>{}) {
		auto&& [p1] = object;
		return std::tie(p1);
	}
	else {
		return std::tie();
	}
}

template<typename ...Types>
std::function<void(Stream&)> MakeTupleSerializer(std::tuple<Types...>& t) {
	return [t](Stream& s) {
		std::apply([&s](auto&&... args) {
			((s.Write(args)), ...);
		}, t);
	};
}

template<typename ...Types>
std::function<void(Stream&)> MakeTupleDeserializer(std::tuple<Types...>& t) {
	return [t](Stream& s) {
		std::apply([&s](Types&&... args) {
			((args = s.Read<std::remove_reference<Types>::type>()), ...);
		}, t);
	};
}

template<typename T>
std::function<void(Stream&)> MakeClassSerializer(T& o) {
	auto t = to_tuple(o);
	return MakeTupleSerializer(t);
}

template<typename T>
std::function<void(Stream&)> MakeClassDeserializer(T& o) {
	auto t = to_tuple(o);
	return MakeTupleDeserializer(t);
}