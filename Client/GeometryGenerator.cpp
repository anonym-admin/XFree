#include "pch.h"
#include "GeometryGenerator.h"

namespace GeometryGenerator
{
	MeshData MakeTriangle()
	{
		MeshData meshData = {};

		Vertex triangleVertices[] =
		{
			{ Vector3(0.0f, 0.25f, 0.0f), Vector4(1.0f, 0.0f, 0.0f, 1.0f) },
			{ Vector3{ 0.25f, -0.25f, 0.0f }, Vector4{ 0.0f, 1.0f, 0.0f, 1.0f } },
			{ Vector3{ -0.25f, -0.25f, 0.0f }, Vector4{ 0.0f, 0.0f, 1.0f, 1.0f } }
		};

		uint32 size = sizeof(triangleVertices);
		uint32 count = size / sizeof(Vertex);

		meshData.vertices = new Vertex[count];
		meshData.verticesSize = size;
		meshData.verticesCount = count;

		::memcpy(meshData.vertices, triangleVertices, sizeof(Vertex) * count);

		uint32 triangleIndices[] = { 0, 1, 2 };

		size = sizeof(triangleIndices);
		count = size / sizeof(Index);

		meshData.indices = new Index[count];
		meshData.indicesSize = size;
		meshData.indicesCount = count;

		::memcpy(meshData.indices, triangleIndices, sizeof(Index) * count);

		return meshData;
	}

	MeshData MakeSqaure(const float scale)
	{
		MeshData meshData = {};

		Vertex squareVertices[] =
		{
			{ Vector3(-1.0f, -1.0f, 0.0f) * scale, Vector4(1.0f, 0.0f, 0.0f, 1.0f)},
			{ Vector3(-1.0f, 1.0f, 0.0f) * scale, Vector4(0.0f, 1.0f, 0.0f, 1.0f)},
			{ Vector3(1.0f, 1.0f, 0.0f) * scale, Vector4(0.0f, 1.0f, 1.0f, 1.0f)},
			{ Vector3(1.0f, -1.0f, 0.0f) * scale, Vector4(0.0f, 0.0f, 1.0f, 1.0f)}
		};

		uint32 size = sizeof(squareVertices);
		uint32 count = size / sizeof(Vertex);

		meshData.vertices = new Vertex[count];
		meshData.verticesSize = size;
		meshData.verticesCount = count;

		::memcpy(meshData.vertices, squareVertices, sizeof(Vertex) * count);

		uint32 squareIndices[] = { 0, 1, 3, 1, 2, 3 };

		size = sizeof(squareIndices);
		count = size / sizeof(Index);

		meshData.indices = new Index[count];
		meshData.indicesSize = size;
		meshData.indicesCount = count;

		::memcpy(meshData.indices, squareIndices, sizeof(Index) * count);

		return meshData;
	}

	MeshData MakeBox(const float scale)
	{
		MeshData meshData = {};

		Vertex squareVertices[24] = {};
		squareVertices[0].posModel	= Vector3(-1.0f, 1.0f, -1.0f) * scale;
		squareVertices[1].posModel	= Vector3(-1.0f, 1.0f, 1.0f) * scale;
		squareVertices[2].posModel	= Vector3(1.0f, 1.0f, 1.0f) * scale;
		squareVertices[3].posModel	= Vector3(1.0f, 1.0f, -1.0f) * scale;
		squareVertices[0].color		= Vector4(1.0f, 0.0f, 0.0f, 1.0f);
		squareVertices[1].color		= Vector4(1.0f, 0.0f, 0.0f, 1.0f);
		squareVertices[2].color		= Vector4(1.0f, 0.0f, 0.0f, 1.0f);
		squareVertices[3].color		= Vector4(1.0f, 0.0f, 0.0f, 1.0f);

		squareVertices[4].posModel	= Vector3(-1.0f, -1.0f, -1.0f) * scale;
		squareVertices[5].posModel	= Vector3(1.0f, -1.0f, -1.0f) * scale;
		squareVertices[6].posModel	= Vector3(1.0f, -1.0f, 1.0f) * scale;
		squareVertices[7].posModel	= Vector3(-1.0f, -1.0f, 1.0f) * scale;
		squareVertices[4].color		= Vector4(1.0f, 0.0f, 0.0f, 1.0f);
		squareVertices[5].color		= Vector4(1.0f, 0.0f, 0.0f, 1.0f);
		squareVertices[6].color		= Vector4(1.0f, 0.0f, 0.0f, 1.0f);
		squareVertices[7].color		= Vector4(1.0f, 0.0f, 0.0f, 1.0f);

		squareVertices[8].posModel	= Vector3(-1.0f, -1.0f, -1.0f) * scale;
		squareVertices[9].posModel	= Vector3(-1.0f, 1.0f, -1.0f) * scale;
		squareVertices[10].posModel = Vector3(1.0f, 1.0f, -1.0f) * scale;
		squareVertices[11].posModel = Vector3(1.0f, -1.0f, -1.0f) * scale;
		squareVertices[8].color		= Vector4(0.0f, 0.0f, 1.0f, 1.0f);
		squareVertices[9].color		= Vector4(0.0f, 0.0f, 1.0f, 1.0f);
		squareVertices[10].color	= Vector4(0.0f, 0.0f, 1.0f, 1.0f);
		squareVertices[11].color	= Vector4(0.0f, 0.0f, 1.0f, 1.0f);

		squareVertices[12].posModel = Vector3(-1.0f, -1.0f, 1.0f) * scale;
		squareVertices[13].posModel = Vector3(1.0f, -1.0f, 1.0f) * scale;
		squareVertices[14].posModel = Vector3(1.0f, 1.0f, 1.0f) * scale;
		squareVertices[15].posModel = Vector3(-1.0f, 1.0f, 1.0f) * scale;
		squareVertices[12].color	= Vector4(0.0f, 1.0f, 1.0f, 1.0f);
		squareVertices[13].color	= Vector4(0.0f, 1.0f, 1.0f, 1.0f);
		squareVertices[14].color	= Vector4(0.0f, 1.0f, 1.0f, 1.0f);
		squareVertices[15].color	= Vector4(0.0f, 1.0f, 1.0f, 1.0f);

		squareVertices[16].posModel = Vector3(-1.0f, -1.0f, 1.0f) * scale;
		squareVertices[17].posModel = Vector3(-1.0f, 1.0f, 1.0f) * scale;
		squareVertices[18].posModel = Vector3(-1.0f, 1.0f, -1.0f) * scale;
		squareVertices[19].posModel = Vector3(-1.0f, -1.0f, -1.0f) * scale;
		squareVertices[16].color	= Vector4(1.0f, 1.0f, 0.0f, 1.0f);
		squareVertices[17].color	= Vector4(1.0f, 1.0f, 0.0f, 1.0f);
		squareVertices[18].color	= Vector4(1.0f, 1.0f, 0.0f, 1.0f);
		squareVertices[19].color	= Vector4(1.0f, 1.0f, 0.0f, 1.0f);

		squareVertices[20].posModel = Vector3(1.0f, -1.0f, 1.0f) * scale;
		squareVertices[21].posModel = Vector3(1.0f, -1.0f, -1.0f) * scale;
		squareVertices[22].posModel = Vector3(1.0f, 1.0f, -1.0f) * scale;
		squareVertices[23].posModel = Vector3(1.0f, 1.0f, 1.0f) * scale;
		squareVertices[20].color	= Vector4(1.0f, 0.0f, 1.0f, 1.0f);
		squareVertices[21].color	= Vector4(1.0f, 0.0f, 1.0f, 1.0f);
		squareVertices[22].color	= Vector4(1.0f, 0.0f, 1.0f, 1.0f);
		squareVertices[23].color	= Vector4(1.0f, 0.0f, 1.0f, 1.0f);


		uint32 size = sizeof(squareVertices);
		uint32 count = size / sizeof(Vertex);

		meshData.vertices = new Vertex[count];
		meshData.verticesSize = size;
		meshData.verticesCount = count;

		::memcpy(meshData.vertices, squareVertices, sizeof(Vertex) * count);

		uint32 squareIndices[] = {
					0,  1,  2,  0,  2,  3,  // À­¸é
					4,  5,  6,  4,  6,  7,  // ¾Æ·§¸é
					8,  9,  10, 8,  10, 11, // ¾Õ¸é
					12, 13, 14, 12, 14, 15, // µÞ¸é
					16, 17, 18, 16, 18, 19, // ¿ÞÂÊ
					20, 21, 22, 20, 22, 23  // ¿À¸¥ÂÊ
		};

		size = sizeof(squareIndices);
		count = size / sizeof(Index);

		meshData.indices = new Index[count];
		meshData.indicesSize = size;
		meshData.indicesCount = count;

		::memcpy(meshData.indices, squareIndices, sizeof(Index) * count);

		return meshData;
	}
}

