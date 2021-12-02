#include "LayerStack.h"

void LayerStack::PushLayer(Ref<Layer> layer)
{
	_layers.push_back(layer);
	layer->OnAttach();
}

void LayerStack::PopLayer(Ref<Layer> layer)
{
	//TODO: Proper Popping
	_layers.pop_back();
	layer->OnDetach();
}

void LayerStack::PushOverlay(Ref<Layer> layer)
{
	//TODO: PushOverlay
}

void LayerStack::PopOverlay(Ref<Layer> layer)
{
	//TODO: PopOverlay
}
