#pragma once

/*
=====================
D3D12CommandList
=====================
*/

class D3D12CommandList
{
public:
	void Init(ID3D12Device* device);
	void Clean();
	void Reset();
	void ExcuteCommandLists();
	void WaitForGpu();

public:
	ID3D12Device* m_device = nullptr;
	ID3D12CommandQueue* m_commandQueue = nullptr;
	ID3D12CommandAllocator* m_commandAllocator = nullptr;
	ID3D12GraphicsCommandList* m_commandList = nullptr;
	ID3D12Fence* m_fence = nullptr;
	HANDLE m_fenceEvent = nullptr;
	uint64 m_fenceValue = 0;

private:
	void CreateCommandAllocatorAndList();
	void CreateFence();
};

