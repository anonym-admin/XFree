#pragma once

#include "../Common/Vertex.h"

/*
==================
GeometryGenerator.
==================
*/

namespace GeometryGenerator
{
	MeshData MakeTriangle();
	MeshData MakeSqaure(const float scale = 1.0f);
	MeshData MakeBox(const float scale = 1.0f);
}
