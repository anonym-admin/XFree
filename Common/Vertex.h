#pragma once

#include <directxtk/SimpleMath.h>
#include <d3d12.h>

/*
======
Math
======
*/

using DirectX::SimpleMath::Vector4;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Matrix;

/*
=======
Vertex
=======
*/
struct Vertex
{
	Vector3 posModel = {};
	Vector4 color = {};
	Vector2 texCoord = {};
};
/*
=======
Index
=======
*/
typedef uint32 Index;

/*
==========
Mesh Data
==========
*/

struct MeshData
{
	Vertex* vertices = nullptr;
	Index* indices = nullptr;
	uint32 verticesCount = 0;
	uint32 indicesCount = 0;
	uint32 verticesSize = 0;
	uint32 indicesSize = 0;
};