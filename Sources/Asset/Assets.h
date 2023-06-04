#pragma once

#include "Core/Module.h"
#include "Core/Core.h"
#include "IO/Stream.h"

#include <filesystem>
#include <unordered_map>
#include <string>
#include <functional>

// Represent as hashed path asset
using AssetGUID = uint64_t;

#define ASSET(name, ext) \
inline static const bool Registered = Register<name>(#ext); \
virtual Type GetType() { return TypeOf<name>(); } \

// Should be inherited by assets classes
//
// when creating a new Asset override virtual OnCreate() to create runtime data with the serialized one
class Asset {
	friend class Assets; // To set _GUID when loaded or created
	
	int _RefCount{ 0 };
	AssetGUID _GUID{ NullGUID };

protected:
	friend class AssetSerializer;
	using TAssetFactory = std::unordered_map<std::string, std::function<Asset*()>>;
	static TAssetFactory& Factories() {
		static TAssetFactory factories;
		return factories;
	}

	template<typename T>
	static bool Register(std::string type) {
		CHECK(Factories().find(type) == Factories().end()); // Check for already registered file type
		Factories().emplace(type, []() { return new T(); });
		return true;
	}

public:
	inline static constexpr AssetGUID NullGUID = 0;

	inline int GetRefCount() { return _RefCount; }
	inline void IncRef() { _RefCount++; }
	inline void DecRef() {
		_RefCount--;
		if (_RefCount == 0) {
			delete this;
		}
	}

	AssetGUID GetGUID() { return _GUID; }

	//Called when a the asset have loaded all data
	virtual void OnLoad(){}
	virtual void Serialize(Stream& S) { CHECK(0); }
	virtual Type GetType() { return TypeOf<Asset>(); };
};


// Points to an existing asset
// Uses reference counting for memory managment
class AssetRef {
protected:
	Asset* _Asset{ nullptr };

public:
	AssetRef(){}
	AssetRef(Asset* asset) : _Asset(asset) {
		_Asset->IncRef();
	}
	~AssetRef() {
		if (_Asset != nullptr) {
			_Asset->DecRef(); 
		}
	}
	
	AssetRef(const AssetRef& other) {
		_Asset = other._Asset;
		if(_Asset != nullptr)_Asset->IncRef();
	}
	
	AssetRef& operator=(const AssetRef& other) {
		Asset* lastAsset = _Asset;

		_Asset = other._Asset;
		if (_Asset != nullptr) {
			_Asset->IncRef();
		}

		if (lastAsset != nullptr) {
			lastAsset->DecRef();
		}

		return *this;
	}
	AssetRef& operator=(Asset* other) {
		Asset* lastAsset = _Asset;

		_Asset = other;
		if (_Asset != nullptr) {
			_Asset->IncRef();
		}

		if (lastAsset != nullptr) {
			lastAsset->DecRef();
		}

		return *this;
	}
	
	inline bool IsValid() { return _Asset != nullptr; }

	Asset* operator ->() {
		CHECK(_Asset != nullptr);
		return _Asset;
	}
};

// The same as AssetRef but with type
template<class T, typename = std::enable_if<std::is_base_of_v<Asset, T>>>
class AssetRefT : public AssetRef {
public:
	AssetRefT() { }
	AssetRefT(AssetRef& assetRef) : AssetRef(assetRef) {
		CHECK(assetRef->GetType() == TypeOf<T>());
	}
	AssetRefT(AssetRef&& assetRef) : AssetRef(assetRef) {
		CHECK(assetRef->GetType() == TypeOf<T>());
	}
	AssetRefT(T* asset) : AssetRef(asset) { }

	T* operator ->() {
		return (T*)_Asset;
	}
};

// Global class to load and manage Assets
class Assets : public ModuleDef<Assets> {
	std::unordered_map<AssetGUID, AssetRef> _AssetsCache;

	static AssetGUID _GenerateGUID(void* seed) {
		std::hash<void*> h;
		return h(seed);
	}

	AssetRef _Load(AssetGUID GUID);


public:

	static AssetGUID Hash(const std::string& assetPath) {
		std::hash<std::string> h = {};
		return h(assetPath);
	}

	// Create a asset in a file in the @path
	// after created the Asset._GUID will be defined
	// also caches it
	// [Editor Only]
	static void CreateAsset(AssetRef asset, const std::string& path);

	// Saves a already existing asset 
	// [Editor Only]
	static void SaveAsset(AssetRef asset);

	// Delete an existing file asset
	// [Editor Only]
	static void DeleteAsset(AssetGUID guid);

	//Load an asset from any Mod by its GUID
	static AssetRef Load(AssetGUID GUID) { return Get()._Load(GUID); }
	//Load an asset from any Mod by its name relative to Mods folder
	static AssetRef Load(const std::string& name) { return Get()._Load(Hash(name)); }

	// Tries to free memory
	// It only frees memory that are mapped to files (with GUID)
	// Runtime assets are not freed
	static void GarbageCollect() {
		//TODO: Try to clean assets that are not used Both Runtime and Loaded ones
		CHECK(0);
	}
};

class AssetSerializer {
public:
	static AssetRef Load(Stream& S) {
		std::filesystem::path p = std::filesystem::path(S.GetIdentifier());
		std::string ext = p.extension().string().substr(1);
		AssetRef a = Asset::Factories()[ext]();
		a->Serialize(S);
		a->OnLoad();
		return a;
	}
	static void Save(AssetRef asset, Stream& S) {
		asset->Serialize(S);
	}
};


// Used to reference an Asset that has a Mod/Path even though it may be not loaded
// Also used to save the asset reference
template<class T, typename = std::enable_if<std::is_base_of_v<Asset, T>>>
class AssetSlot : public AssetRefT<T> {
	AssetGUID _GUID{ Asset::NullGUID };

	void _load() {
		CHECK(AssetRef::_Asset == nullptr);
		CHECK(_GUID != Asset::NullGUID);

		AssetRef::operator=(Assets::Load(_GUID));
		if (!AssetRef::IsValid()) {
			Log::error("Trying to load a invalid asset GUID: {}", GetAsHex());
			_GUID = Asset::NullGUID;
		}
	}

	inline T* _get() {
		if (_GUID != Asset::NullGUID && !AssetRef::IsValid()) {
			_load();
		}
		return (T*)AssetRef::_Asset;
	}

public:

	AssetSlot() {}
	AssetSlot(AssetRefT<T>& Ref) : AssetRefT<T>(Ref), _GUID(Ref->GetGUID()) {}
	AssetSlot(AssetGUID GUID) : _GUID(GUID) {}

	AssetGUID GetGUID() {
		return _GUID;
	}
	void SetGUID(AssetGUID GUID) {
		_GUID = GUID;
		AssetRef::operator=(nullptr);
	}

	inline T* operator ->() {
		CHECK(AssetRef::IsValid());
		return _get();
	}

	inline bool IsValid() {
		T* ptr = _get();
		return ptr != nullptr && ptr->GetGUID() == _GUID;
	}

	std::string GetAsHex() { return fmt::format("{0:X}", _GUID); }
};

