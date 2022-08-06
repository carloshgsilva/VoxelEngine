#include "VoxImporter.h"

#include "IO/Log.h"
#include "Asset/PalleteAsset.h"
#include "Asset/PrefabAsset.h"
#include "World/Components.h"
#include "World/World.h"

#include <map>
#include <vector>
#include <charconv>

//TODO: The importer should create the .asset file

inline static const bool _Debug = false;
inline static const bool _Debug_Nodes_Creation = false;


/*
store a row-major rotation in the bits of a byte

for example :
R =
 0  1  0
 0  0 -1
-1  0  0
==>
unsigned char _r = (1 << 0) | (2 << 2) | (0 << 4) | (1 << 5) | (1 << 6)

bit | value
0-1 : 1 : index of the non-zero entry in the first row
2-3 : 2 : index of the non-zero entry in the second row
4   : 0 : the sign in the first row (0 : positive; 1 : negative)
5   : 1 : the sign in the second row (0 : positive; 1 : negative)
6   : 1 : the sign in the third row (0 : positive; 1 : negative)
*/
struct VoxTransformMatrix {
	uint8 _r;
	uint8 _rx, _ry, _rz;
	uint8 _sx, _sy, _sz;

	VoxTransformMatrix() : VoxTransformMatrix(0b0100) {
	}

	VoxTransformMatrix(uint8 r) {
		_r = r;

		//Map Axes
		_rx = (_r >> 0) & 0b11;
		_ry = (_r >> 2) & 0b11;
		_rz = (3 - (_rx | _ry));

		//Signal 1 if negative
		_sx = (_r >> 4) & 1;
		_sy = (_r >> 5) & 1;
		_sz = (_r >> 6) & 1;
	}
	inline int getFromVec(int idx, glm::ivec3& vec) {
		switch (idx) {
		case 0: return vec.x; break;
		case 1: return vec.y; break;
		case 2: return vec.z; break;
		default: CHECK(0);
		}
	}
	glm::ivec3 transform(glm::ivec3 vec, glm::ivec3& size) {
		int x = getFromVec(_rx, vec);
		int y = getFromVec(_ry, vec);
		int z = getFromVec(_rz, vec);

		return glm::ivec3(
			_sx ? (getFromVec(_rx, size) - x - 1) : x,
			_sy ? (getFromVec(_ry, size) - y - 1) : y,
			_sz ? (getFromVec(_rz, size) - z - 1) : z
		);
	}
	glm::ivec3 transformSize(glm::ivec3& size) {
		return glm::ivec3(
			getFromVec(_rx, size),
			getFromVec(_ry, size),
			getFromVec(_rz, size)
		);
	}
};

class VoxNode { public: virtual ~VoxNode() {} };
class VoxGroup : public VoxNode { public: std::vector<int> childrenIds; };
class VoxTransform : public VoxNode { public: int x, y, z, childId; VoxTransformMatrix matrix; std::string name; };
class VoxShape : public VoxNode { public: int shapeId; };

struct VoxShapeData {
	int sizeX, sizeY, sizeZ;
	std::vector<uint32_t> voxels;
};

class VoxImportContext {
public:
	std::filesystem::path path;
	std::filesystem::path fileName;

	World* world;
	AssetRefT<PalleteAsset> assetPallete;

	std::string nodeName;
	std::vector<std::shared_ptr<VoxNode>> nodes;
	std::vector<std::shared_ptr<VoxShapeData>> shapesData;
	struct VoxColor {
		uint8 r, g, b, a;
	};
	struct VoxSurface {
		uint8 e, r, m;//emit, roughness, metallic
	};
	VoxColor pallete[257];
	VoxSurface surfaces[257];
	uint8 remap[256];

	int sizeX = 0, sizeY = 0, sizeZ = 0;

	std::map<std::string, std::string> readDictionary(Stream& s) {
		std::map<std::string, std::string> m;
		int size = 0;
		s | size;
		if constexpr (_Debug)Log::info("  Dictionary<>({})", size);
		for (int i = 0; i < size; i++) {
			std::string key;
			std::string value;
			s | key | value;
			if constexpr (_Debug)Log::info("      <{}, {}>", key, value);
			m[key] = value;
		}
		return m;
	}

	void readRGBA(Stream& s) {
		if constexpr (_Debug)Log::info("RGBA");
		s.Serialize(&pallete[1], 256 * 4);
	}

	void readXYZI(Stream& s) {
		int voxelCount;

		s | voxelCount;
		if constexpr (_Debug)Log::info("XYZI ({})", voxelCount);

		auto shapeData = std::make_shared<VoxShapeData>();
		shapeData->voxels.resize(voxelCount);

		s.Serialize(shapeData->voxels.data(), shapeData->voxels.size() * sizeof(uint32_t));

		shapeData->sizeX = sizeX;
		shapeData->sizeY = sizeY;
		shapeData->sizeZ = sizeZ;

		shapesData.push_back(shapeData);

	}

	void readMATL(Stream& s) {
		int materialId;
		s | materialId;//materialId

		if constexpr (_Debug)Log::info("MATL {}", materialId);

		auto properties = readDictionary(s);//materialProperties

		float rough = 0.0f;
		float emit = 0.0f;
		float metal = 0.0f;

		std::string type = properties["_type"];

		if (type == "_metal" || type == "_blend") {
			if (properties.find("_rough") != properties.end()) {
				auto& prop = properties["_rough"];
				std::from_chars(prop.data(), prop.data() + prop.size(), rough);
			}
			if (properties.find("_metal") != properties.end()) {
				auto& prop = properties["_metal"];
				std::from_chars(prop.data(), prop.data() + prop.size(), metal);
			}

		}
		else if (type == "_emit") {
			if (properties.find("_emit") != properties.end()) {
				auto& prop = properties["_emit"];
				std::from_chars(prop.data(), prop.data() + prop.size(), emit);
			}
		}
		//_type == _diffuse have the default value of _rough to 0.1 which is strange so just put 0.9
		else if (type == "_diffuse") {
			rough = 0.9f;
		}

		surfaces[materialId].e = emit * 255.0f;
		surfaces[materialId].r = rough * 255.0f;
		surfaces[materialId].m = metal * 255.0f;
	}

	void readIMAP(Stream& s) {
		s.Serialize(remap, 256);
	}

	void readnTRN(Stream& s) {
		if constexpr (_Debug)Log::info("nTRN");
		int tempInt;

		std::shared_ptr<VoxTransform> transform = std::make_shared<VoxTransform>();

		transform->name = nodeName;

		s | transform->childId; //childNodeId
		s | tempInt; //reservedId
		s | tempInt; //layerId
		s | tempInt; //numOfFrame

		auto map = readDictionary(s);

		int beginT = 0;
		int endT = 0;
		auto str = map["_t"];

		int x, y, z;
		x = y = z = 0;
		{//Read _t = Translation
			if (!str.empty()) {
				endT = str.find(' ', beginT + 1) + 1;
				std::from_chars(str.data() + beginT, str.data() + endT, x);
				beginT = endT;
				endT = str.find(' ', beginT + 1) + 1;
				std::from_chars(str.data() + beginT, str.data() + endT, y);
				beginT = endT;
				endT = str.size();
				std::from_chars(str.data() + beginT, str.data() + endT, z);
				transform->x = x;
				transform->y = y;
				transform->z = z;
			}
		}

		int _r = 0b0100;
		{//Read _r = Rotation
			str = map["_r"];

			if (!str.empty()) {
				std::from_chars(str.data(), str.data() + str.size(), _r);
			}
		}

		transform->matrix = VoxTransformMatrix(static_cast<uint8>(_r));

		nodes.push_back(transform);
	}

	void readnGRP(Stream& s) {
		if constexpr (_Debug)Log::info("nGRP");

		auto group = std::make_shared<VoxGroup>();

		int size;

		s | size;
		group->childrenIds.resize(size);
		for (int i = 0; i < size; i++) { s | group->childrenIds[i]; }

		nodes.push_back(group);
	}

	void readnSHP(Stream& s) {
		if constexpr (_Debug)Log::info("nSHP");

		auto shape = std::make_shared<VoxShape>();
		int tempInt;

		s | tempInt;//numOfChildren is always 1
		s | shape->shapeId;//childId

		nodes.push_back(shape);

		readDictionary(s);//reserved (not used)

	}


	bool Import(Stream& s) {
		if constexpr (_Debug)Log::info("Importing a vox file!");

		//VOX Header
		char VOX_[4];
		s.Serialize(VOX_, sizeof(VOX_));

		if (std::memcmp(VOX_, "VOX ", 4)) { Log::warn("Trying to load an invalid vox file! {}", s.GetIdentifier()); return false; }

		//Version
		int version = 0;
		s | version;

		int tempInt;

		std::string str;
		std::map<std::string, std::string> map;
		int x, y, z;
		int beginT = 0, endT = 0;

		std::map<std::string, std::string> nodeDict;

		bool running = true;
		while (running) {
			char Header[4];
			Header[0] = Header[1] = Header[2] = Header[3] = ' ';
			s.Serialize(Header, sizeof(Header));

			int chunkContentSize = 0;
			int childrenChunkContentSize = 0;
			s | chunkContentSize;
			s | childrenChunkContentSize;

			//if constexpr (_Debug)Log::info("Chunk Size {} Children Size {}", chunkContentSize, childrenChunkContentSize);

			int nodeId = -1;
			int size = 0;

			switch (Header[0]) {
			case 'M': //MAIN
				switch (Header[2]) {
				case 'I':
					if constexpr (_Debug)Log::info("MAIN");
					break;
				case 'T':
					readMATL(s);
					break;
				}

				break;

			case 'P': //PACK
				s | tempInt;
				if constexpr (_Debug)Log::info("PACK {}", tempInt);
				break;

			case 'S': //SIZE
				s | sizeX | sizeY | sizeZ;
				if constexpr (_Debug)Log::info("SIZE {} {} {}", sizeX, sizeY, sizeZ);
				break;

			case 'X': //XYZI (Blocks)
				readXYZI(s);
				break;

			case 'R': //RGBA (Pallete)
				readRGBA(s);
				break;

			case 'L': //LAYR (Layer)
				if constexpr (_Debug)Log::info("LAYR");
				s | tempInt;//layerId
				readDictionary(s);//layerAttributes
				s | tempInt;//Reserved
				break;

			case 'I': //IMAP (Vox Index Remap)
				if constexpr (_Debug)Log::info("IMAP");
				readIMAP(s);
				break;

			case 'r': //rOBJ
				if constexpr (_Debug)Log::info("rOBJ");
				readDictionary(s);//objectAttributes
				break;

			case 'n': //n (Node)
				s | nodeId;
				nodeDict = readDictionary(s); //nodeAttributes
				nodeName = nodeDict.find("_name") != nodeDict.end() ? nodeDict["_name"] : "";
				switch (Header[1]) {
				case('T'): //nTRN
					readnTRN(s);
					break;
				case('G'): //nGRP (Group of Nodes)
					readnGRP(s);
					break;
				case('S'): //nSHP (Connect a Node to a Shape)
					readnSHP(s);
					break;
				}
				break;
			default:
				running = false;
				//Log::info("Finish {}{}{}{}", Header[0], Header[1], Header[2], Header[3]);
				break;
			}

		}
		return true;
	}
};

static int TEMP_counter = 0;
static entt::entity CreateEntity(VoxImportContext& ctx, std::shared_ptr<VoxTransform> root) {
	std::shared_ptr<VoxNode> node = ctx.nodes[root->childId];
	if constexpr (_Debug_Nodes_Creation)Log::info("Transform(");

	//Create Entity
	entt::entity e = ctx.world->Create();

	//Add Transform Component
	Transform t{};
	t.Position = glm::vec3(root->x, root->z, -root->y) * 0.1f;//Flip-Z-Axis

	//Group
	std::shared_ptr<VoxGroup> group = std::dynamic_pointer_cast<VoxGroup>(node);
	if (group.get() != nullptr) {
		//emplace parent Transform Component
		ctx.world->GetRegistry().emplace<Transform>(e, t);

		if constexpr (_Debug_Nodes_Creation)Log::info("Group[{}](", group->childrenIds.size());
		for (int childId : group->childrenIds) {
			std::shared_ptr<VoxTransform> childTransformNode = std::dynamic_pointer_cast<VoxTransform>(ctx.nodes[childId]);

			entt::entity child = CreateEntity(ctx, childTransformNode);
			ctx.world->SetParent(child, e);
		}
		if constexpr (_Debug_Nodes_Creation)Log::info(")");

		return e;
	}

	//Shape
	std::shared_ptr<VoxShape> shape = std::dynamic_pointer_cast<VoxShape>(node);
	if (shape != nullptr) {
		std::shared_ptr<VoxShapeData> shapeData = ctx.shapesData[shape->shapeId];
		if constexpr (_Debug_Nodes_Creation)Log::info("Shape()");



		glm::ivec3 size = glm::ivec3(shapeData->sizeX, shapeData->sizeY, shapeData->sizeZ);
		glm::ivec3 tSize = root->matrix.transformSize(size);//Transformed Size

		AssetRefT<VoxAsset> vox = new VoxAsset(tSize.x, tSize.z, tSize.y);

		{//Offset center 
			glm::ivec3 center = glm::ivec3(
				root->matrix._sx ? tSize.x - tSize.x / 2 : tSize.x / 2,
				root->matrix._sz ? tSize.z - tSize.z / 2 : tSize.z / 2,
				!root->matrix._sy ? tSize.y - tSize.y / 2 : tSize.y / 2//Flip-Z-Axis
			);

			t.Position -= glm::vec3(center) * 0.1f;
		}


		ctx.world->GetRegistry().emplace<Transform>(e, t);

		for (uint32 data : shapeData->voxels) {
			uint8 x = (data & 0x000000FF) >> (0 * 8);
			uint8 y = (data & 0x0000FF00) >> (1 * 8);
			uint8 z = (data & 0x00FF0000) >> (2 * 8);
			uint8 v = (data & 0xFF000000) >> (3 * 8);

			glm::ivec3 coords = root->matrix.transform(glm::ivec3(x, y, z), size);
			*vox->PixelAt(coords.x, coords.z, tSize.y - 1 - coords.y) = v;//Flip-Z-Axis
		}
		vox->Upload();

		std::string shapeName = root->name.empty() ? fmt::format("{}", TEMP_counter++) : root->name;
		ctx.world->SetName(e, shapeName);
		Assets::CreateAsset(vox, (ctx.path / ctx.fileName / shapeName).replace_extension("v").generic_string());

		//Add VoxRenderer Component
		VoxRenderer vr;
		vr.Pallete = ctx.assetPallete;
		vr.Vox = vox;
		ctx.world->GetRegistry().emplace<VoxRenderer>(e, vr);

		return e;
	}

	if constexpr (_Debug_Nodes_Creation)Log::info(")");

	return entt::null;
}

void VoxImporter::Import(Stream& s) {
	TEMP_counter = 0;

	//Serialize ctx
	VoxImportContext ctx;
	ctx.Import(s);

	//Create World
	Unique<World> W = NewUnique<World>();

	//Create Pallete
	AssetRefT<PalleteAsset> newPallete = new PalleteAsset();
	for (int i = 0; i < 256; i++) {
		VoxMaterial& Mat = newPallete->MaterialAt(i);
		Mat.r = ctx.pallete[i].r;
		Mat.g = ctx.pallete[i].g;
		Mat.b = ctx.pallete[i].b;
		Mat.emit = ctx.surfaces[i].e;
		Mat.roughness = ctx.surfaces[i].r;
		Mat.metallic = ctx.surfaces[i].m;
	}
	newPallete->Upload();
	Assets::CreateAsset(newPallete, (_Path / _FileName / _FileName).replace_extension("p").generic_string());
	
	ctx.world = W.get();
	ctx.assetPallete = newPallete;
	ctx.path = _Path;
	ctx.fileName = _FileName;

	//Create entities
	std::shared_ptr<VoxTransform> rootTransform = std::dynamic_pointer_cast<VoxTransform>(ctx.nodes[0]);
	entt::entity root = CreateEntity(ctx, rootTransform);
	W->SetName(root, _FileName.generic_string());

	//Create Prefab
	AssetRefT<PrefabAsset> prefab = new PrefabAsset();
	prefab->FromWorld(W.get(), root);
	Assets::CreateAsset(prefab, (_Path / _FileName.replace_extension("pf")).generic_string());
}