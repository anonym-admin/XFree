#pragma once

#include "../Common/Vertex.h"

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

	D3D12Mesh* CreateMesh(MeshData meshData);
	void RenderMesh(D3D12Mesh* mesh);
	void DestroyMesh(D3D12Mesh* mesh);

private:
	const static uint32 s_FrameCount = 2;

	// Pipeline objects.
	D3D12_VIEWPORT m_viewport = {};
	D3D12_RECT m_scissorRect = {};
	IDXGISwapChain3* m_swapChain = nullptr;
	ID3D12Device* m_device = nullptr;
	ID3D12Resource* m_renderTargets[s_FrameCount] = {};
	ID3D12Resource* m_depthStencilBuffer = nullptr;
	ID3D12CommandAllocator* m_commandAllocator = nullptr;
	ID3D12CommandQueue* m_commandQueue = nullptr;
	ID3D12DescriptorHeap* m_rtvHeap = nullptr;
	ID3D12DescriptorHeap* m_dsvHeap = nullptr;
	ID3D12GraphicsCommandList* m_commandList = nullptr;
	uint32 m_rtvDescriptorSize = 0;
	uint32 m_dsvDesciptorSize = 0;

	// Synchronization objects.
	uint32 m_frameIndex = 0;
	HANDLE m_fenceEvent = nullptr;
	ID3D12Fence* m_fence = nullptr;
	uint64 m_fenceValue = 0;

	bool m_useWarpDevice = true;
	HWND m_hwnd = nullptr;

	float m_screenWidth = 0.0f;
	float m_screenHeight = 0.0f;
	float m_aspectRatio = 0.0f;

	void CreateDescriptorHeap();
	void CreateFrameResources();
	void CreateCommandAllocatorAndList();
	void CreateFence();

	void DestroyDesriptorHeap();
	void DestroyFrameResources();
	void DestroyCommandAllocatorAndList();
	void DestroyFence();

	void WaitForPreviousFrame();
};

