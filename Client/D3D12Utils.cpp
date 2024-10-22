#include "pch.h"
#include "D3D12Utils.h"
#include "D3D12CommandList.h"
#include <directxtk12/DDSTextureLoader.h>
#include <directxtk12/ResourceUploadBatch.h>

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

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
		auto desc = CD3DX12_RESOURCE_DESC::Buffer(size);
		ThrowIfFailed(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&vertexBuffer->resource)));

		heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		desc = CD3DX12_RESOURCE_DESC::Buffer(size);
		ThrowIfFailed(device->CreateCommittedResource(&heapProps,D3D12_HEAP_FLAG_NONE,&desc,D3D12_RESOURCE_STATE_GENERIC_READ,nullptr,IID_PPV_ARGS(&vertexBuffer->upload)));

		// Describe the data we want to copy into the default buffer.
		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = vertices;
		subResourceData.RowPitch = size;
		subResourceData.SlicePitch = subResourceData.RowPitch;

		// TODO
		D3D12CommandList* ctx = new D3D12CommandList;
		ctx->Init(device);
		ctx->Reset();

		ctx->m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer->resource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
		UpdateSubresources<1>(ctx->m_commandList, vertexBuffer->resource, vertexBuffer->upload, 0, 0, 1, &subResourceData);
		ctx->m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer->resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

		ctx->ExcuteCommandLists();
		ctx->WaitForGpu();
		ctx->Clean();

		delete ctx;
		ctx = nullptr;

		// Initialize the vertex buffer view.
		vertexBuffer->vertexBufferView.BufferLocation = vertexBuffer->resource->GetGPUVirtualAddress();
		vertexBuffer->vertexBufferView.StrideInBytes = stride;
		vertexBuffer->vertexBufferView.SizeInBytes = size;

		return vertexBuffer;
	}

	IndexBuffer* CreateIndexBuffer(ID3D12Device* device, void* indices, uint32 count, uint32 size, DXGI_FORMAT format)
	{
		IndexBuffer* indexBuffer = new IndexBuffer;

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
		auto desc = CD3DX12_RESOURCE_DESC::Buffer(size);
		ThrowIfFailed(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&indexBuffer->resource)));

		heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		desc = CD3DX12_RESOURCE_DESC::Buffer(size);
		ThrowIfFailed(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexBuffer->upload)));

		// Describe the data we want to copy into the default buffer.
		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = indices;
		subResourceData.RowPitch = size;
		subResourceData.SlicePitch = subResourceData.RowPitch;

		// TODO
		D3D12CommandList* ctx = new D3D12CommandList;
		ctx->Init(device);
		ctx->Reset();

		ctx->m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer->resource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
		UpdateSubresources<1>(ctx->m_commandList, indexBuffer->resource, indexBuffer->upload, 0, 0, 1, &subResourceData);
		ctx->m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer->resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

		ctx->ExcuteCommandLists();
		ctx->WaitForGpu();
		ctx->Clean();

		delete ctx;
		ctx = nullptr;

		// Initialize the vertex buffer view.
		indexBuffer->indexBufferView.BufferLocation = indexBuffer->resource->GetGPUVirtualAddress();
		indexBuffer->indexBufferView.Format = format;
		indexBuffer->indexBufferView.SizeInBytes = size;

		return indexBuffer;
	}

	TextureHandle* CreateTexture2D(ID3D12Device* device, const wchar_t* filename, D3D12_CPU_DESCRIPTOR_HANDLE srvHandle)
	{
		using namespace DirectX;

		// TODO
		D3D12CommandList* ctx = new D3D12CommandList;
		ctx->Init(device);

		TextureHandle* textureHandle = new TextureHandle;
		ResourceUploadBatch resourceUpload(device);
		resourceUpload.Begin();

		ThrowIfFailed(CreateDDSTextureFromFile(device, resourceUpload, filename, &textureHandle->resource));

		auto uploadResourcesFinished = resourceUpload.End(ctx->m_commandQueue);

		uploadResourcesFinished.wait();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = textureHandle->resource->GetDesc().Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = textureHandle->resource->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		device->CreateShaderResourceView(textureHandle->resource, &srvDesc, srvHandle);
		textureHandle->srvHandle = srvHandle;

		ctx->Clean();
		delete ctx;
		ctx = nullptr;

		return textureHandle;
	}

	uint32 CalcConstantBufferByteSize(uint32 size)
	{
		return (size + 255) & ~255;
	}
}