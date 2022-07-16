#pragma once

#include "Core.h"
#include "Layer/Layer.h"

#include <vector>

class LayerStack {

public:
	void PushLayer(Ref<Layer> layer);
	void PopLayer(Ref<Layer> layer);
	void PushOverlay(Ref<Layer> layer);
	void PopOverlay(Ref<Layer> layer);

	std::vector<Ref<Layer>>::iterator begin() { return _layers.begin(); }
	std::vector<Ref<Layer>>::iterator end() { return _layers.end(); }

	void Clear() {
		_layers.clear();
	}

private:
	std::vector<Ref<Layer>> _layers;
};