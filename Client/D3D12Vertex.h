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

/*
=======
Vertex
=======
*/

struct Vertex
{
	Vector3 posModel = {};
	Vector4 color = {};
};

struct VertexBuffer
{
	ID3D12Resource* resource = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
};

struct IndexBuffer
{
	ID3D12Resource* resource = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
};