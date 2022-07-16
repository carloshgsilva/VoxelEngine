#pragma once

#include <string>
#include <vector>
#include <sstream>

namespace voxpp {

	struct Matrix {
		uint8_t _r;
		uint8_t _rx, _ry, _rz;
		uint8_t _sx, _sy, _sz;

		Matrix() : Matrix(0b0100) {
		}

		Matrix(uint8_t r) {
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
		inline int getFromVec(int idx, Vec3 v) {
			switch (idx) {
			case 0: return v.x;
			case 1: return v.y;
			case 2: return v.z;
			}
			return v.z;
		}
		Vec3 transform(Vec3 vec, Vec3& size) {
			int x = getFromVec(_rx, vec);
			int y = getFromVec(_ry, vec);
			int z = getFromVec(_rz, vec);

			return Vec3(
				_sx ? (getFromVec(_rx, size) - x - 1) : x,
				_sy ? (getFromVec(_ry, size) - y - 1) : y,
				_sz ? (getFromVec(_rz, size) - z - 1) : z
			);
		}
		Vec3 transformSize(Vec3& size) {
			return Vec3(
				getFromVec(_rx, size),
				getFromVec(_ry, size),
				getFromVec(_rz, size)
			);
		}
	};


	struct Node { virtual ~Node() {} };
	struct Transform : public Node {
		int x, y, z, layer;
		Matrix matrix;
		int childId;
	};
	struct Group : public Node {
		std::vector<int> childrenIds;
	};
	struct Instance : public Node {
		Matrix matrix;
		int sizex, sizey, sizez;
		std::vector<uint8_t> Voxels;
	};

	struct Vec3 {
		int x, y, z;
		Vec3(int x, int y, int z) : x(x), y(y), z(z) {

		}
	};
	
	struct Material {
		uint8_t r, g, b, a;
		uint8_t emissive, roughness, metallic;
	};


	class Scene {
		int _sizex, _sizey, _sizez;
		uint8_t* _data;
		Transform _trnNode;

		struct Shape {
			struct Block {
				uint8_t x, y, z, v;
			};
			int sizex, sizey, sizez;
			std::vector<Block> Blocks;
		};
		std::vector<Shape> _shapes;

		struct DictnTRN {
			int x, y, z;
			int r;
		};
		struct DictMATL {
			float roughness;
			float emit;
			float metal;
			float ior;
		};

		int readInt() {
			int v = *(int*)_data;
			_data += sizeof(int);
			return v;
		}

		std::string readString() {
			int size = *(int*)_data; _data += 4;
			char* ptr = (char*)_data; _data += size;
			return std::string(ptr, size);
		}

		void readDictSKIP() {
			int size = *(int*)_data; _data += 4;
			for (int i = 0; i < size; i++) {
				int l = *(int*)_data; _data += 4;
				_data += l;
				l = *(int*)_data; _data += 4;
				_data += l;
			}
		}
		DictnTRN readDictnTRN() {
			DictnTRN d{};

			int size = readInt();
			for (int i = 0; i < size; i++) {
				std::string key = readString();
				std::stringstream ssvalue(readString());
				switch (key[1])
				{
				case 't':
					ssvalue >> d.x;
					ssvalue >> d.y;
					ssvalue >> d.z;
					break;
				case 'r':
					ssvalue >> d.r;
					break;
				default:
					break;
				}
			}

			return d;
		}
		DictMATL readDictMATL() {
			DictMATL d{};

			int size = readInt();
			for (int i = 0; i < size; i++) {
				std::string key = readString();
				std::stringstream ssvalue(readString());
				switch (key[1])
				{
				case 'r':
					ssvalue >> d.roughness;
					break;
				case 'm':
					ssvalue >> d.metal;
					break;
				case 'e':
					ssvalue >> d.emit;
					break;
				case 'i':
					ssvalue >> d.ior;
					break;
				default:
					break;
				}
			}

			return d;
		}

		void readRGBA() { memcpy(pallete, _data, 256 * 4); _data += 256 * 4; }
		void readSIZE() {
			_sizex = readInt();
			_sizey = readInt();
			_sizez = readInt();
		}
		void readXYZI() {
			size_t blocksCount = readInt();

			_shapes.push_back({});
			Shape& shape = _shapes.back();
			shape.sizex = _sizex;
			shape.sizey = _sizey;
			shape.sizez = _sizez;
			shape.Blocks.resize(blocksCount);

			memcpy(shape.Blocks.data(), _data, blocksCount * 4);
			_data += blocksCount * 4;
		}
		void readMATL() {
			int materialId = readInt();

			DictMATL props = readDictMATL();

			Material& m = pallete[materialId];
			m.roughness = props.roughness;
			m.emissive = props.emit;
			m.metallic = props.metal;
		}
		void readIMAP() { _data += 256; } // Not used for now
		void readnTRN() {
			readInt(); // childId
			readInt(); // reservedId
			int layer = readInt();
			readInt(); //int numOfFrame;

			DictnTRN d = readDictnTRN();
			std::unique_ptr<Transform> trn = std::make_unique<Transform>();
			trn->x = d.x;
			trn->y = d.y;
			trn->z = d.z;
			trn->matrix = Matrix(d.r);
			trn->layer = layer;

			nodes.push_back(std::move(trn));
			//All the nTRN nodes are proceded by a nSHP or nGRP
		}

		void readnGRP() {
			std::unique_ptr<Group> group = std::make_unique<Group>();
			int size = readInt();
			group->childrenIds.resize(size);
			for (int i = 0; i < size; i++) {
				group->childrenIds[i] = readInt(); // childId
			}
			nodes.push_back(std::move(group));
		}
		void readnSHP() {
			std::unique_ptr<Instance> shape = std::make_unique<Instance>();

			readInt();//numOfChildren is always 1
			shape->shapeId = readInt();//shapeId
			nodes.push_back(std::move(shape));
		}
	public:
		std::vector<std::unique_ptr<Node>> nodes;
		Material pallete[256];

		static Scene FromMemory(void* data, size_t size);
		static Scene FromFile(const std::string& path);
	};

}