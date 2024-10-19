#pragma once

#include "D3D12Vertex.h"

/*
==================
D3D12Renderer
==================
*/

class D3D12Mesh;

class D3D12Renderer
{
public:
	bool Init(HWND hwnd);
	void Clean();
	void Update();
	void BeginRender();
	void EndRender();
	void Present();

	D3D12Mesh* CreateMesh(Vertex* vertices, uint32 verticesSize, uint32* indices, uint32 indicesSize);
	void RenderMesh(D3D12Mesh* mesh);
	void DestroyMesh(D3D12Mesh* mesh);

	// TEMP
	ID3D12GraphicsCommandList* GetCommandList() { return m_commandList; }
	
private:
	const static uint32 s_FrameCount = 2;

	// Pipeline objects.
	D3D12_VIEWPORT m_viewport = {};
	D3D12_RECT m_scissorRect = {};
	IDXGISwapChain3* m_swapChain = nullptr;
	ID3D12Device* m_device = nullptr;
	ID3D12Resource* m_renderTargets[s_FrameCount] = {};
	ID3D12CommandAllocator* m_commandAllocator = nullptr;
	ID3D12CommandQueue* m_commandQueue = nullptr;
	ID3D12DescriptorHeap* m_rtvHeap = nullptr;
	ID3D12GraphicsCommandList* m_commandList = nullptr;
	uint32 m_rtvDescriptorSize = 0;

	// Synchronization objects.
	uint32 m_frameIndex = 0;
	HANDLE m_fenceEvent = nullptr;
	ID3D12Fence* m_fence = nullptr;
	uint64 m_fenceValue = 0;

	bool m_useWarpDevice = true;
	HWND m_hwnd = nullptr;

	float m_aspectRatio = 0.0f;

	void CreateDescriptorHeap();
	void CreateFrameResources();
	void CreateCommandAllocatorAndList();
	void CreateFence();

	void WaitForPreviousFrame();
};

/*
================
D3D12Mesh
================
*/

class D3D12Mesh
{
public:
	bool Init(ID3D12Device* device, Vertex* vertices, uint32 vertexSize, uint32* indices, uint32 indexSize);
	void Clean();
	void Update();
	void Render(ID3D12GraphicsCommandList* commandList);

private:
	ID3D12Device* m_device = nullptr;
	ID3D12RootSignature* m_rootSignature = nullptr;
	ID3D12PipelineState* m_pipelineState = nullptr;
	// App resources.
	VertexBuffer* m_vertexBuffer = nullptr;
};