#define ARC_TOOLS

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <filesystem>

#include "ArcGlobals.h"
#include "engine/Resource.h"
#include "util/Geometry.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

void LoadModel(const std::string &filename, std::vector<Vertex> &vertices, std::vector<u32> &indices)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	std::unordered_map<Vertex, u32> uniqueVertices = {};

	ARC_ASSERT(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str()));

	for (const auto &shape : shapes)
	{
		for (const auto &index : shape.mesh.indices)
		{
			Vertex vertex = {};

			vertex.pos =
			{
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord =
			{
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<u32>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}
}

void ExportModel(const std::string &filename, std::vector<Vertex> &vertices, std::vector<u32> &indices)
{
	std::ofstream outputFile;
	outputFile.open(filename, std::ios::out | std::ios::binary | std::ios::trunc);

	ResourceHeader header;

	u32 vertexCount = static_cast<u32>(vertices.size());
	u32 indexCount = static_cast<u32>(indices.size());
	u64 vertexDataSize = sizeof(Vertex) * vertexCount;
	u64 indexDataSize = sizeof(u32) * indexCount;

	u64 fileSize = 2 * sizeof(u32) + vertexDataSize + indexDataSize;

	// Header
	memcpy(header.mSignature, "ARCR", 4);
	header.mType = RESOURCETYPE_GRAPHIC;
	header.mSize = fileSize;
	outputFile.write(reinterpret_cast<const char *>(&header), sizeof(header));
	
	outputFile.write(reinterpret_cast<const char *>(&vertexCount), sizeof(u32));
	outputFile.write(reinterpret_cast<const char *>(&indexCount), sizeof(u32));

	outputFile.write(reinterpret_cast<const char *>(vertices.data()), vertexDataSize);
	outputFile.write(reinterpret_cast<const char *>(indices.data()), indexDataSize);

	outputFile.close();
}

int main(int argc, char **argv)
{
	ARC_UNUSED(argc);
	ARC_UNUSED(argv);
	const std::string path = "models";
	for (const auto &entry : std::filesystem::directory_iterator(path))
	{
		std::vector<Vertex> vertices;
		std::vector<u32> indices;

		std::filesystem::path outputPath = entry.path();
		outputPath.replace_extension("bin");

		LoadModel(entry.path().string(), vertices, indices);
		ExportModel(outputPath.string(), vertices, indices);
	}
}
