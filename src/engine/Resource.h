#pragma once

#include "ArcGlobals.h"

enum ResourceType
{
	RESOURCETYPE_GRAPHIC,
	RESOURCETYPE_IMAGE
};

struct ResourceHeader
{
	char mSignature[4];
	ResourceType mType;
	u64 mSize;
};

class Resource
{
	friend class ResourceManager;

	virtual void Load(void *data, u64 dataSize) = 0;
};
