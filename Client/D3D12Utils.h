#pragma once

#include "../Common/Vertex.h"

/*
================
D3D12Utils
================
*/

void ThrowIfFailed(HRESULT hr);

namespace D3D12Utils
{

	 VertexBuffer* CreateVertexBuffer(ID3D12Device* device, void* vertices, uint32 count, uint32 size, uint32 stride);
	 IndexBuffer* CreateIndexBuffer(ID3D12Device* device, void* indices, uint32 count, uint32 size, DXGI_FORMAT format);
}

