#include "ComponentManager.h"

const std::vector<GraphicComponent>::const_iterator ComponentManager::GraphicComponentsBegin() const
{
	return mGraphicComponents.begin();
}

const std::vector<GraphicComponent>::const_iterator ComponentManager::GraphicComponentsEnd() const
{
	return mGraphicComponents.end();
}

const GraphicComponent &ComponentManager::CreateGraphicComponent(std::string resourceFilename)
{
	mGraphicComponents.push_back(GraphicComponent { mResourceManager->LoadGraphicResource(resourceFilename) });
	return mGraphicComponents[mGraphicComponents.size() - 1];
}
