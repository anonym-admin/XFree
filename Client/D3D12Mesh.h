#pragma once

#include "../Common/Vertex.h"

struct ConstBufferData
{
	Matrix world;
	Matrix view;
	Matrix proj;
};

struct TextureHandle;
class D3D12Renderer;

/*
================
D3D12Mesh.
================
*/

class D3D12Mesh
{
public:
	bool Init(D3D12Renderer* device, MeshData meshData);
	void Clean();
	void UpdateWorldMatrix(Matrix worldRow);
	void Update();
	void Render(ID3D12GraphicsCommandList* commandList);

private:
	static uint32 sm_refCount;
	static ID3D12RootSignature* sm_rootSignature;
	static ID3D12PipelineState* sm_pipelineState;

	D3D12Renderer* m_renderer = nullptr;
	// App resources.
	VertexBuffer* m_vertexBuffer = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;

	ID3D12DescriptorHeap* m_cbvHeap = nullptr;
	uint32 m_cbvsrvDesciptorSize = 0;

	ConstBufferData m_constData = {};
	ID3D12Resource* m_constBuffer = nullptr;
	BYTE* m_mappedData = nullptr;

	MeshData m_meshData = {};

	Matrix m_worldRow = Matrix();

	TextureHandle* m_textureHandle = nullptr;

	void CreateRootSignature();
	void CreatePipelineState();
	void CreateDesciptorHeap();
	void CreateConstantBuffer();
	void CreateTextureResource();

	void DestroyRootSignature();
	void DestroyPipelineState();
	void DestroyDescriptorHeap();
	void DestroyConstantBuffer();
	void DestroyTextureResource();
};