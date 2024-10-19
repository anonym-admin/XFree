#pragma once

#include "../Common/Vertex.h"

struct ConstBufferData
{
	Matrix world;
	Matrix view;
	Matrix proj;
};

class D3D12Renderer;

/*
================
D3D12Mesh
================
*/

class D3D12Mesh
{
public:
	bool Init(D3D12Renderer* device, MeshData meshData);
	void Clean();
	void Update();
	void Render(ID3D12GraphicsCommandList* commandList);

private:
	D3D12Renderer* m_renderer = nullptr;
	ID3D12RootSignature* m_rootSignature = nullptr;
	ID3D12PipelineState* m_pipelineState = nullptr;
	// App resources.
	VertexBuffer* m_vertexBuffer = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;

	ID3D12DescriptorHeap* m_cbvHeap = nullptr;
	uint32 m_cbvsrvDesciptorSize = 0;

	ConstBufferData m_constData = {};
	ID3D12Resource* m_constBuffer = nullptr;
	BYTE* m_mappedData = nullptr;

	MeshData m_meshData = {};

	void CreateRootSignature();
	void CreatePipelineState();
	void CreateDesciptorHeap();
	void CreateConstantBuffer();

	void DestroyRootSignature();
	void DestroyPipelineState();
	void DestroyDescriptorHeap();
	void DestroyConstantBuffer();
};