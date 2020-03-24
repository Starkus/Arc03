#include "ComponentManager.h"

ComponentManager *ComponentManager::sInstance;

void ComponentManager::Initialize()
{
	static ComponentManager instance;
	sInstance = &instance;
}

const GraphicComponent &ComponentManager::CreateGraphicComponent(std::string resourceFilename)
{
	GraphicResource *resource = static_cast<GraphicResource *>(ResourceManager::Instance()->LoadResource(resourceFilename));
	mGraphicComponents.push_back(GraphicComponent { resource });
	return mGraphicComponents[mGraphicComponents.size() - 1];
}
