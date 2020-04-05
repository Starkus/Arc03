#include "ComponentManager.h"

ComponentManager *ComponentManager::sInstance;

void ComponentManager::Initialize()
{
	static ComponentManager instance;
	sInstance = &instance;
}

const GraphicComponent &ComponentManager::CreateGraphicComponent(std::string resourceFilename)
{
	Resource *const resource = ResourceManager::Instance()->LoadResource(resourceFilename);
	GraphicResource *graphicResource = static_cast<GraphicResource *>(resource);
	mGraphicComponents.push_back(GraphicComponent { graphicResource });
	return mGraphicComponents[mGraphicComponents.size() - 1];
}
