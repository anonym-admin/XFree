#pragma once

#include "../Common/Vertex.h"

namespace GeometryGenerator
{
	MeshData MakeTriangle();
	MeshData MakeSqaure(const float scale = 1.0f);
	MeshData MakeBox(const float scale = 1.0f);
}
