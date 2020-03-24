#pragma once

#include "ArcGlobals.h"
#include "ResourceManager.h"

#include <vector>
#include <string>

struct GraphicComponent
{
	GraphicResource *mGraphicResource;
};

class ComponentManager
{
	ARC_DEFINE_SINGLETON(ComponentManager);
protected:
	ComponentManager() = default;

private:
	std::vector<GraphicComponent> mGraphicComponents;

public:
	static void Initialize();
	const std::vector<GraphicComponent>::const_iterator GraphicComponentsBegin() const { return mGraphicComponents.begin(); }
	const std::vector<GraphicComponent>::const_iterator GraphicComponentsEnd() const { return mGraphicComponents.end(); }
	const GraphicComponent &CreateGraphicComponent(std::string resourceFilename);
};
