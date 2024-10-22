#pragma once

#include <d3d12.h>

/*
===========
GPU Buffer
===========
*/

struct VertexBuffer
{
	ID3D12Resource* resource = nullptr;
	ID3D12Resource* upload = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	uint32 count = 0;
};

struct IndexBuffer
{
	ID3D12Resource* resource = nullptr;
	ID3D12Resource* upload = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
	uint32 count = 0;
};