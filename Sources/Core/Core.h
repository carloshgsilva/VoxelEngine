#pragma once

#include <memory>
#include "IO/Log.h"

/////////
// Ref //
/////////

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using WeakRef = std::weak_ptr<T>;


template<typename T, typename ... Args>
constexpr Ref<T> New(Args&& ... args) {
	return std::make_shared<T>(std::forward<Args>(args)...);
}

////////////
// Unique //
////////////

template<typename T>
using Unique = std::unique_ptr<T>;

template<typename T, typename ... Args>
constexpr Unique<T> NewUnique(Args&& ... args) {
	return std::move(std::make_unique<T>(std::forward<Args>(args)...));
}

//////////
// Type //
//////////


struct TypeDef {
	bool IsClass;
	bool IsClassPtr;
	bool IsFundamental;
	size_t Size;
};

template<typename T>
struct Internal_TypeId {
	static TypeDef Def;
};
template<typename T>
TypeDef Internal_TypeId<T>::Def = {
	std::is_base_of_v<class Object, T>,
	std::is_base_of_v<class Object, std::remove_pointer_t<T>> && std::is_pointer_v<T>,
	std::is_fundamental_v<T>
};

using Type = TypeDef*;


template<typename T>
static const constexpr Type TypeOf() {
	return &Internal_TypeId<T>::Def;
}

/////////////////
// Value Types //
/////////////////

using int8 = int8_t;
using uint8 = uint8_t;
using int32 = int32_t;
using int16 = int16_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

#include <assert.h>

/////////////
// Asserts //
/////////////

#define CHECK(expression) \
if ((!(expression))) { \
Log::error("Check Error: {} Line: {}" , __FILE__, __LINE__); throw std::exception(__FILE__); \
} \
