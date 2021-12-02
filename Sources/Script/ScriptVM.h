#pragma once

#include "Core/Core.h"

#include <wren/wren.hpp>
#include <cstddef>
#include <functional>
#include <optional>

namespace ScriptUtil {
    inline std::size_t hashMethodSignature(
        const char* module,
        const char* className,
        bool isStatic,
        const char* signature)
    {
        std::hash<std::string> hash;
        std::string qualified(module);
        qualified += className;
        qualified += signature;
        if (isStatic)
        {
            qualified += 's';
        }
        return hash(qualified);
    }
    inline std::size_t hashClassSignature(
        const char* module,
        const char* className)
    {
        std::hash<std::string> hash;
        std::string qualified(module);
        qualified += className;
        return hash(qualified);
    }
}

struct Handle {
    WrenVM* vm{ nullptr };
    WrenHandle* handle{ nullptr };

    void Release() {
        if (vm != nullptr && handle != nullptr) {
            wrenReleaseHandle(vm, handle);
            vm = nullptr;
            handle = nullptr;
        }
    }

    Handle() : vm(nullptr), handle(nullptr) {}
    Handle(WrenVM* vm, WrenHandle* handle) : vm(vm), handle(handle) {}
    Handle(const Handle& other) {
        vm = other.vm;
        if (vm == nullptr)return;
        //Creates other handle as it is a copy operator
        wrenEnsureSlots(vm, 1);
        wrenSetSlotHandle(vm, 0, other.handle);
        handle = wrenGetSlotHandle(vm, 0);
    }
    Handle(Handle&&) = default; // Enable move
    ~Handle() { Release(); }
    
    Handle& operator=(Handle&& other) noexcept {
        Release();
        vm = other.vm;
        handle = other.handle;
        other.vm = nullptr;
        other.handle = nullptr;
        return *this;
    };
    Handle& operator =(const Handle& other) {
        Release();
        vm = other.vm;
        //Creates other handle as it is a copy operator
        wrenEnsureSlots(vm, 1);
        wrenSetSlotHandle(vm, 0, other.handle);
        handle = wrenGetSlotHandle(vm, 0);
        return *this;
    };
};

class ScriptVM {
public:
    WrenVM* vm;
    void* user_data;

    std::unordered_map<size_t, WrenForeignMethodFn> _ForeignMethods;
    std::unordered_map<size_t, WrenForeignClassMethods> _ForeignClasses;

    void RegisterFunction(const char* moduleName, const char* className, bool isStatic, const char* signature, WrenForeignMethodFn func) {
        _ForeignMethods.emplace(
            ScriptUtil::hashMethodSignature(moduleName, className, isStatic, signature),
            func
        );
    }
    void RegisterClass(const char* moduleName, const char* className, WrenForeignClassMethods clsMtds) {
        _ForeignClasses.emplace(
            ScriptUtil::hashClassSignature(moduleName, className),
            clsMtds
        );
    }
    struct ModuleContext;

    template<typename Base>
    struct ForeignClassContext {
        ModuleContext* _module;
        const char* _name;
        int size;

        using ForeignMethod = std::function<void(WrenVM*, Base*)>;

        template <typename Sig, Sig sig> struct MemberFieldDetail;
        template<typename T, T Base::* Field> struct MemberFieldDetail<T Base::*, Field> { using FieldType = T; };
        
        ForeignClassContext& Ctor() {
            WrenForeignClassMethods m{};
            m.allocate = [](WrenVM* vm) {
                Base* data = (Base*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(Base));
                *data = Base();
            };
            m.finalize = [](void* ptr) {
                Base* data = (Base*)ptr;
                data->~Base();
            };
            _module->_context->RegisterClass(_module->_name, _name, m);
            return *this;
        }

        template<auto Field>
        ForeignClassContext& Field(const char* name) {
            using FieldType = typename MemberFieldDetail<decltype(Field), Field>::FieldType;
            // Get
            Method(name, [](WrenVM* vm) {
                Base* Obj = (Base*)wrenGetSlotForeign(vm, 0);
                if constexpr (std::is_same_v<FieldType, float> || std::is_same_v<FieldType, double> || std::is_same_v<FieldType, int>) {
                    wrenSetSlotDouble(vm, 0, Obj->*Field);
                }
                else if constexpr (std::is_same_v<FieldType, std::string>) {
                    wrenSetSlotString(vm, 0, Obj->*Field.c_str());
                }
                else if constexpr (std::is_same_v<FieldType, bool>) {
                    wrenSetSlotBool(vm, 0, Obj->*Field);
                }
                else {
                    static_assert(false, "Failed to resolve Var type");
                }
            });

            // Set
            Method((std::string(name) + "=(_)").c_str(), [](WrenVM* vm) {
                Base* Obj = (Base*)wrenGetSlotForeign(vm, 0);
                if constexpr (std::is_same_v<FieldType, float> || std::is_same_v<FieldType, double> || std::is_same_v<FieldType, int>) {
                    Obj->*Field = wrenGetSlotDouble(vm, 1);
                }
                else if constexpr (std::is_same_v<FieldType, std::string>) {
                    Obj->*Field = wrenGetSlotString(vm, 1);
                }
                else if constexpr (std::is_same_v<FieldType, bool>) {
                    Obj->*Field = wrenGetSlotBool(vm, 1);
                }
                else {
                    static_assert(false, "Failed to resolve Var type");
                }
            });

            return *this;
        }

        ForeignClassContext& Method(const char* signature, WrenForeignMethodFn func) {
            _module->_context->RegisterFunction(_module->_name, _name, false, signature, func);
            return *this;
        }

        ForeignClassContext& StaticMethod(const char* sig, WrenForeignMethodFn func) {
            _module->_context->RegisterFunction(_module->_name, _name, true, sig, func);
            return *this;
        }

        ModuleContext& End() { return *_module; }
    };

    struct ClassContext {
        ModuleContext* _module;
        const char* _name;

        ClassContext& Method(const char* sig, WrenForeignMethodFn func) {
            _module->_context->RegisterFunction(_module->_name, _name, false, sig, func);
            return *this;
        }

        ClassContext& StaticMethod(const char* sig, WrenForeignMethodFn func) {
            _module->_context->RegisterFunction(_module->_name, _name, true, sig, func);
            return *this;
        }

        ModuleContext& End() { return *_module; }
    };

    struct ModuleContext {
        ScriptVM* _context;
        const char* _name;

        template<typename T>
        ForeignClassContext<T> Class(const char* name) { return ForeignClassContext<T>{ this, name, sizeof(T) }; }
        
        ClassContext Class(const char* name) { return ClassContext{ this, name }; }
    };

	ScriptVM();
    ~ScriptVM();

    void Initialize();
    void Destroy();

    void Reload();

    void Run(const std::string& code);
    bool HasModule(const std::string& path);
    void RunModule(const std::string& path);


    template<typename T> Handle NewForeign(Handle& handle, T data) {
        wrenSetSlotHandle(vm, 0, handle.handle);
        T* ptr = (T*)wrenSetSlotNewForeign(vm, 1, 0, sizeof(T));
        *ptr = data;
        return Handle(vm, wrenGetSlotHandle(vm, 1));
    }
    Handle GetVariable(const std::string& moduleName, const std::string& variableName) {
        wrenEnsureSlots(vm, 1);
        wrenGetVariable(vm, moduleName.c_str(), variableName.c_str(), 0);
        return Handle( vm, wrenGetSlotHandle(vm, 0) );
    }
    Handle GetMethod(const std::string& signature) {
        return Handle(vm, wrenMakeCallHandle(vm, signature.c_str()));
    }


    void Set(int slot, double& v) { wrenSetSlotDouble(vm, slot, v); }
    void Set(int slot, Handle& v) { wrenSetSlotHandle(vm, slot, v.handle); }
    template<typename T> void Set(int slot, Handle& foreign, T& v) { wrenSetSlotDouble(vm, slot, v); }

    double GetDouble(int slot = 0) { return wrenGetSlotDouble(vm, slot); }
    Handle GetHandle(int slot = 0) { return Handle(vm, wrenGetSlotHandle(vm, slot)); }
    template<typename T> T* GetForeign(int slot = 0) { return (T*)wrenGetSlotForeign(vm, slot); }

    // Makes a call to a object
    // if returned true you can get the return value by calling Get****() methods
    template<typename ...Args> inline bool Call(Handle& instance, Handle& method, Args...args) {
        wrenEnsureSlots(vm, 1 + sizeof...(Args));
        wrenSetSlotHandle(vm, 0, instance.handle);
        int idx = 1;
        (Set(idx++, args), ...);
        WrenInterpretResult r = wrenCall(vm, method.handle);
        return r == WrenInterpretResult::WREN_RESULT_SUCCESS;
    }

    ModuleContext Module(const char* name) {
        return ModuleContext{this, name};
    }

    
    void SetUserData(void* ptr) { user_data = ptr; }
    template<class T> T* GetUserData() { return (T*)user_data; }

    static ScriptVM* From(WrenVM* vm) { return (ScriptVM*)wrenGetUserData(vm); }
};