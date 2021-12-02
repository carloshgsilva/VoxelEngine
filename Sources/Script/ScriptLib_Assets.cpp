#include "ScriptLib_Assets.h"

#include "Asset/PalleteAsset.h"

//TODO: better solution as how are we going to handle creating with parameters?
//		Solution.1 actually call an method inside the wren implementation
//		Eg. construct new(x, y, z) { initialize(x, y, z) {} }

//TODO:: Replace with a templated solution to remove duplication, call it ##ForeignAsset##
struct ScriptHandle_PalleteAsset {
	PalleteAsset* Ptr{nullptr};
	ScriptHandle_PalleteAsset() {
		Ptr = new PalleteAsset();
		Ptr->IncRef();
	}
	~ScriptHandle_PalleteAsset() {
		Ptr->DecRef();
	}
};

void ScriptLib_Assets::Import(ScriptVM& context) {
	context.Module("Assets")
		.Class<ScriptHandle_PalleteAsset>("PalleteAsset")
			.Ctor()
			.Method("upload()", [](WrenVM* vm) {
				ScriptHandle_PalleteAsset* asset = (ScriptHandle_PalleteAsset*)wrenGetSlotForeign(vm, 0);
				if (asset->Ptr == nullptr)return;

				asset->Ptr->Upload();
			})
			.Method("set(_,_,_,_)", [](WrenVM* vm) {
				ScriptHandle_PalleteAsset* asset = (ScriptHandle_PalleteAsset*)wrenGetSlotForeign(vm, 0);
				if (asset->Ptr == nullptr)return;

				int idx = wrenGetSlotDouble(vm, 1);
				int r = wrenGetSlotDouble(vm, 2);
				int g = wrenGetSlotDouble(vm, 3);
				int b = wrenGetSlotDouble(vm, 4);

				VoxMaterial& mat = asset->Ptr->MaterialAt(idx);
				mat.r = r;
				mat.g = r;
				mat.b = b;
			})
		.End();
}
