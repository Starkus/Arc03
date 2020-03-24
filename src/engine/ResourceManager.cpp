#include "ResourceManager.h"

#include <iostream>
#include <fstream>

#include "render/VulkanEngine.h"

ResourceManager *ResourceManager::sInstance;

void ResourceManager::Initialize()
{
	static ResourceManager instance;
	sInstance = &instance;

	sInstance->mGraphicResources.reserve(1024); // TODO: allocate these properly
}

Resource *ResourceManager::AllocateResource(ResourceHeader header)
{
	switch (header.mType)
	{
		case RESOURCETYPE_GRAPHIC:
		{
			mGraphicResources.resize(mGraphicResources.size() + 1);
			return &mGraphicResources.back();
		} break;
	};
	return nullptr;
}

Resource *ResourceManager::LoadResource(std::string filename)
{
	std::ifstream file;
	file.open(filename.c_str(), std::ios::binary);

	ResourceHeader header;
	file.read(reinterpret_cast<char *>(&header), sizeof(header));

	void *data = malloc(header.mSize);
	file.read(reinterpret_cast<char *>(data), header.mSize);

	file.close();

	Resource *resource = AllocateResource(header);
	resource->Load(data, header.mSize);

	free(data);
	return resource;
}
