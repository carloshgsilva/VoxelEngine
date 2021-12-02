#pragma once

#include "Assets.h"

#include <sstream>
#include <iostream>
#include <json11/json11.hpp>
#include <entt/entt.hpp>

class World;

//TODO: Store the components as a std::map<type, Component>
//		then add with an switch ?
class PrefabAsset : public Asset {
	ASSET(PrefabAsset, pf)

	//Runtime data
	json11::Json _Json;

public:

	entt::entity Spawn(World* W, entt::entity parentEntity = entt::null);
	void FromWorld(World* W, entt::entity entityToSave);

	virtual void Serialize(Stream& S) {

		if (S.IsLoading()) {
			std::string data(S.GetSize(), ' ');
			S.Serialize(data.data(), S.GetSize());
			std::string error;
			_Json = json11::Json::parse(data, error);
		}
		else {
			std::string text = _Json.dump();
			S.Serialize(text.data(), text.size());
		}

	}
};