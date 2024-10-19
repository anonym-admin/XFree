#include "pch.h"
#include "D3D12Utils.h"

/*
================
D3D12Utils
================
*/

void ThrowIfFailed(HRESULT hr)
{
	if(FAILED(hr))
	{
		int32 errorCode = hr;
		__debugbreak();
	}
}

namespace D3D12Utils
{
	VertexBuffer* CreateVertexBuffer(ID3D12Device* device, void* vertices, uint32 count, uint32 size, uint32 stride)
	{
		VertexBuffer* vertexBuffer = new VertexBuffer;

		// Note: using upload heaps to transfer static data like vert buffers is not 
		// recommended. Every time the GPU needs it, the upload heap will be marshalled 
		// over. Please read up on Default Heap usage. An upload heap is used here for 
		// code simplicity and because there are very few verts to actually transfer.
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		auto desc = CD3DX12_RESOURCE_DESC::Buffer(size);
		ThrowIfFailed(device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&vertexBuffer->resource)));

		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		ThrowIfFailed(vertexBuffer->resource->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, vertices, size);
		vertexBuffer->resource->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		vertexBuffer->vertexBufferView.BufferLocation = vertexBuffer->resource->GetGPUVirtualAddress();
		vertexBuffer->vertexBufferView.StrideInBytes = stride;
		vertexBuffer->vertexBufferView.SizeInBytes = size;

		return vertexBuffer;
	}

	IndexBuffer* CreateIndexBuffer(ID3D12Device* device, void* indices, uint32 count, uint32 size, DXGI_FORMAT format)
	{
		IndexBuffer* indexBuffer = new IndexBuffer;

		// Note: using upload heaps to transfer static data like vert buffers is not 
		// recommended. Every time the GPU needs it, the upload heap will be marshalled 
		// over. Please read up on Default Heap usage. An upload heap is used here for 
		// code simplicity and because there are very few verts to actually transfer.
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		auto desc = CD3DX12_RESOURCE_DESC::Buffer(size);
		ThrowIfFailed(device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&indexBuffer->resource)));

		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		ThrowIfFailed(indexBuffer->resource->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, indices, size);
		indexBuffer->resource->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		indexBuffer->indexBufferView.BufferLocation = indexBuffer->resource->GetGPUVirtualAddress();
		indexBuffer->indexBufferView.Format = format;
		indexBuffer->indexBufferView.SizeInBytes = size;

		return indexBuffer;
	}
}