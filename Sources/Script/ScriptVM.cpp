#include "ScriptVM.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>

#include "IO/Log.h"

#include <wren/wren.hpp>
#include <wren/vm/wren_vm.h>

inline static const bool DEBUG_SCRIPT = false;

void write(WrenVM* vm, const char* str) {
	Log::info("[wren] {}", str);
}

WrenLoadModuleResult loadModule(WrenVM* vm, const char* name) {
	std::string path(name);
	path += ".wren";
	std::ifstream fin;
	fin.open(path, std::ios::in);
	std::stringstream buffer;
	buffer << fin.rdbuf() << '\0';
	std::string source = buffer.str();

	char* cbuffer = (char*)malloc(source.size());
	assert(cbuffer);
	memcpy(cbuffer, source.c_str(), source.size());

	WrenLoadModuleResult result{};
	result.source = cbuffer;
	result.userData = cbuffer;
	result.onComplete = [](WrenVM* vm, const char* name, struct WrenLoadModuleResult result) {
		free(result.userData);
	};

	return result;
}

WrenForeignClassMethods bindForignClass(WrenVM* vm, const char* module, const char* className) {
	if(DEBUG_SCRIPT)Log::info("bindForignClass {}:{}", module, className);
	ScriptVM* context = ScriptVM::From(vm);
	
	size_t hashed = ScriptUtil::hashClassSignature(module, className);
	auto it = context->_ForeignClasses.find(hashed);

	if (it != context->_ForeignClasses.end()) {
		return context->_ForeignClasses[hashed];
	}
	else {
		Log::error("[wren] could not find foreign class implementation {}:{}", module, className);
	}

	//Default One
	WrenForeignClassMethods m;
	m.allocate = [](WrenVM* vm) { wrenSetSlotNewForeign(vm, 0, 0, 4); };
	m.finalize = [](void* p) {};

	return m;
}

WrenForeignMethodFn bindForeignMethod(WrenVM* vm,
	const char* module,
	const char* className,
	bool isStatic,
	const char* signature) {
	ScriptVM* context = ScriptVM::From(vm);
	
	if(DEBUG_SCRIPT)Log::info("bindForeignMethod module = {}, className = {}, isStatic = {}, signature = {}", module, className, isStatic, signature);
	
	size_t hashed = ScriptUtil::hashMethodSignature(module, className, isStatic, signature);
	auto it = context->_ForeignMethods.find(hashed);

	if (it != context->_ForeignMethods.end()) {
		return context->_ForeignMethods[hashed];
	}
	else {
		Log::error("[wren] could not find foreign method implementation {}{}:{}.{}", isStatic ? "static " : "", module, className, signature);
	}

	return WrenForeignMethodFn{};
}

void onError(WrenVM* vm, WrenErrorType type, const char* module, int line, const char* message) {
	Log::error("[wren] {} in {} at line {}", message ? message : "", module ? module : "", line ? line : 0);
}

ScriptVM::ScriptVM() {
	Initialize();
}

ScriptVM::~ScriptVM() {
	Destroy();
}

void ScriptVM::Initialize() {
	if (vm != nullptr) {
		Log::warn("Trying to initialize an already initialized ScriptVM!");
	}

	WrenConfiguration config;
	wrenInitConfiguration(&config);
	config.writeFn = write;
	config.bindForeignClassFn = bindForignClass;
	config.bindForeignMethodFn = bindForeignMethod;
	config.loadModuleFn = loadModule;
	config.errorFn = onError;

	vm = wrenNewVM(&config);
	wrenSetUserData(vm, this);
}

void ScriptVM::Destroy() {
	wrenFreeVM(vm);
	vm = nullptr;
}

void ScriptVM::Reload() {
	Destroy();
	Initialize();
}

bool ScriptVM::HasModule(const std::string& path) {
	return wrenHasModule(vm, path.c_str());
}
void ScriptVM::Run(const std::string& code) {
	wrenInterpret(vm, "Run", code.c_str());
}
void ScriptVM::RunModule(const std::string& path) {
	wrenInterpret(vm, "RunModule", fmt::format("import \"{}\" \n ", path).c_str());
}
