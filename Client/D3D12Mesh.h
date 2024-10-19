#pragma once

#include "../Common/Vertex.h"

/*
================
D3D12Mesh
================
*/

class D3D12Mesh
{
public:
	bool Init(ID3D12Device* device, MeshData meshData);
	void Clean();
	void Update();
	void Render(ID3D12GraphicsCommandList* commandList);

private:
	ID3D12Device* m_device = nullptr;
	ID3D12RootSignature* m_rootSignature = nullptr;
	ID3D12PipelineState* m_pipelineState = nullptr;
	// App resources.
	VertexBuffer* m_vertexBuffer = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;
};