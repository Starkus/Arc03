#pragma once

#include "ResourceManager.h"

#include <vector>
#include <string>

struct GraphicComponent
{
	GraphicResource mGraphicResource;
};

class ComponentManager
{
	ResourceManager *const mResourceManager;

	std::vector<GraphicComponent> mGraphicComponents;

public:
	ComponentManager(ResourceManager *resourceManager) : mResourceManager(resourceManager) {}

	const std::vector<GraphicComponent>::const_iterator GraphicComponentsBegin() const;
	const std::vector<GraphicComponent>::const_iterator GraphicComponentsEnd() const;
	const GraphicComponent &CreateGraphicComponent(std::string resourceFilename);
};
