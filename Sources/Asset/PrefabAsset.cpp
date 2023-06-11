#include "PrefabAsset.h"

#include "World/World.h"
#include "World/Components.h"

#include <charconv>

glm::vec3 readVec3(const std::string& str) {
	int s1 = str.find_first_of(' ');
	int s2 = str.find_first_of(' ', s1+1);

	glm::vec3 v;
	std::from_chars(str.data(), str.data() + s1, v.x);
	std::from_chars(str.data() + s1+1, str.data() + s2, v.y);
	std::from_chars(str.data() + s2+1, str.data() + str.size(), v.z);

	return v;
}
std::string writeVec3(const glm::vec3 v) {
	return fmt::format("{} {} {}", v.x, v.y, v.z);
}

//TODO: Put in a good place like Asset::GUIDFromString()
AssetGUID readGUID(const std::string& str) {
	AssetGUID guid;
	std::from_chars(str.data(), str.data() + str.size(), guid, 16);
	return guid;
}

entt::entity PrefabAsset::Spawn(World* W, entt::entity parentEntity) {
	using namespace json11;
	entt::registry& R = W->GetRegistry();

	std::unordered_map<int, entt::entity> newEntityMap;

	entt::entity root = entt::null;

	for (auto& entity : _Json.array_items()) {
		entt::entity parent = entt::null;
		if (!entity["Parent"].is_null()) {
			parent = newEntityMap[entity["Parent"].int_value()];
		}
		
		bool is_root = parent == entt::null;

		entt::entity e;
		//If is a normal entity just create a new one
		if (entity["Instance"].is_null()) {
			e = W->Create();
		}
		//If is an Instance use Prefab.Spawn for the new entity
		else {
			Instance i;
			i.Prefab = readGUID(entity["Instance"].string_value());
			if (i.Prefab.IsValid()) {
				e = i.Prefab->Spawn(W, parent);
			}
			else {
				Log::error("Failed to load invalid instance! {}", i.Prefab.GetAsHex());
			}
		}
		Hierarchy& h = R.get<Hierarchy>(e);

		//TODO: use entity path instead of id
		newEntityMap.emplace(entity["Id"].int_value(), e);

		//Is root
		if (is_root) {
			root = e;
			W->SetParent(e, parentEntity);

			//If is not world root and is an valid file asset
			//Crete a Instance component
			if (parentEntity != entt::null && GetGUID() != Asset::NullGUID) {
				Instance i;
				i.Prefab = GetGUID();
				R.emplace_or_replace<Instance>(e, i);
			}
		}
		else {
			W->SetParent(e, parent);
		}

		//Name
		if (entity["Name"].is_null()) {
			h.Name = fmt::format("Entity {}", (int)R.entity(e));
		}
		else {
			h.Name = entity["Name"].string_value();
		}

		//TODO: Use Hashing instead of strcmp
		for (auto& component : entity.object_items()) {

			if (component.first == "Transform") {
				Transform t;
				t.Position = readVec3(component.second["Position"].string_value());
				t.Rotation = readVec3(component.second["Rotation"].string_value());
				t.Scale = readVec3(component.second["Scale"].string_value());
				R.emplace_or_replace<Transform>(e, t);
			}
			else if (component.first == "VoxRenderer") {
				VoxRenderer r;
				r.Pallete = readGUID(component.second["Pallete"].string_value());
				r.Vox = readGUID(component.second["Vox"].string_value());
				if(!component.second["Pivot"].is_null())r.Pivot = readVec3(component.second["Pivot"].string_value());

				R.emplace_or_replace<VoxRenderer>(e, r);
			}
			else if (component.first == "Light") {
				Light c;
				c.LightType = (Light::Type)(component.second["LightType"].int_value());
				c.Intensity = component.second["Intensity"].number_value();
				c.Color = readVec3(component.second["Color"].string_value());
				c.Attenuation = component.second["Attenuation"].number_value();
				c.Range = component.second["Range"].number_value();
				c.Angle = component.second["Angle"].number_value();
				c.AngleAttenuation = component.second["AngleAttenuation"].number_value();
				R.emplace_or_replace<Light>(e, c);
			}
			else if (component.first == "IKChain") {
				IKChain c;
				c.Target = component.second["Target"].string_value();
				c.Pole = component.second["Pole"].string_value();
				c.Depth = component.second["Depth"].int_value();
				R.emplace_or_replace<IKChain>(e, c);
			}
			else if (component.first == "Script") {
				Script c;
				c.Asset = readGUID(component.second["Asset"].string_value());
				R.emplace_or_replace<Script>(e, c);
			}
			else if (component.first == "Character") {
				Character c;
				R.emplace_or_replace<Character>(e, c);
			}
		}
	}

	return root;
}

void PrefabAsset::FromWorld(World* W, entt::entity entityToSave) {
	entt::registry& R = W->GetRegistry();

	using namespace json11;


	auto entities = Json::array();

	std::function<void(const entt::entity child)> save_entity;

	save_entity = [&](const entt::entity child) {
		bool skipChildren = false;

		Json::object entity;
		entity.emplace("Id", (int)R.entity(child));

		//TODO: manage a way to make variants possible
		if (R.has<Instance>(child) && child != entityToSave) { // don't save Instance for the root
			if (R.get<Instance>(child).Prefab.IsValid()) { // only use Instance if the prefab is a real file
				entity.emplace("Instance", R.get<Instance>(child).Prefab.GetAsHex().c_str());
				skipChildren = true;
			}
		}
		if (R.has<Hierarchy>(child)) {
			auto& n = R.get<Hierarchy>(child);
			entity.emplace("Name", n.Name);
		}
		//Don't save parent of the root
		if (R.has<Hierarchy>(child) && child != entityToSave) {
			entity.emplace("Parent", (int)R.entity(R.get<Hierarchy>(child).Parent));
		}
		if (R.has<Transform>(child)) {
			auto& t = R.get<Transform>(child);
			auto tjson = Json::object();

			tjson.emplace("Position", writeVec3(t.Position).c_str());
			tjson.emplace("Rotation", writeVec3(t.Rotation).c_str());
			tjson.emplace("Scale", writeVec3(t.Scale).c_str());

			entity.emplace("Transform", tjson);
		}
		if (R.has<VoxRenderer>(child)) {
			auto& vr = R.get<VoxRenderer>(child);
			auto vrjson = Json::object();

			vrjson.emplace("Pallete", vr.Pallete.GetAsHex().c_str());
			vrjson.emplace("Vox", vr.Vox.GetAsHex().c_str());
			vrjson.emplace("Pivot", writeVec3(vr.Pivot).c_str());

			entity.emplace("VoxRenderer", vrjson);
		}
		if (R.has<Light>(child)) {
			auto& c = R.get<Light>(child);
			auto vrjson = Json::object();

			vrjson.emplace("LightType", (int)c.LightType);
			vrjson.emplace("Intensity", c.Intensity);
			vrjson.emplace("Color", writeVec3(c.Color));
			vrjson.emplace("Attenuation", c.Attenuation);
			vrjson.emplace("Range", c.Range);
			vrjson.emplace("Angle", c.Angle);
			vrjson.emplace("AngleAttenuation", c.AngleAttenuation);

			entity.emplace("Light", vrjson);
		}
		if (R.has<IKChain>(child)) {
			auto& c = R.get<IKChain>(child);
			auto cjson = Json::object();

			cjson.emplace("Target", c.Target.GetPath());
			cjson.emplace("Pole", c.Pole.GetPath());
			cjson.emplace("Depth", c.Depth);

			entity.emplace("IKChain", cjson);
		}
		if (R.has<Script>(child)) {
			auto& c = R.get<Script>(child);
			auto cjson = Json::object();

			cjson.emplace("Asset", c.Asset.GetAsHex().c_str());

			entity.emplace("Script", cjson);
		}
		if (R.has<Character>(child)) {
			auto cjson = Json::object();
			entity.emplace("Character", cjson);
		}

		entities.push_back(entity);

		if (!skipChildren) {
			W->ForEachChild(child, [&](entt::entity c) {
				save_entity(c);
			});
		}
	};

	save_entity(entityToSave);
	_Json = entities;
}
