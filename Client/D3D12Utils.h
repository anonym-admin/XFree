#pragma once

#include "../Common/Vertex.h"

/*
================
D3D12Utils
================
*/

struct TextureHandle
{
	ID3D12Resource* resource = nullptr;
	ID3D12Resource* upload = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = {};
};

void ThrowIfFailed(HRESULT hr);

namespace D3D12Utils
{
	VertexBuffer* CreateVertexBuffer(ID3D12Device* device, void* vertices, uint32 count, uint32 size, uint32 stride);
	IndexBuffer* CreateIndexBuffer(ID3D12Device* device, void* indices, uint32 count, uint32 size, DXGI_FORMAT format);
	TextureHandle* CreateTexture2D(ID3D12Device* device, const wchar_t* filename, D3D12_CPU_DESCRIPTOR_HANDLE srvHandle);
	uint32 CalcConstantBufferByteSize(uint32 size);
}

