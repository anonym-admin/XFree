#include "pch.h"
#include "D3D12CommandList.h"
#include "D3D12Utils.h"

/*
=====================
D3D12CommandList
=====================
*/

void D3D12CommandList::Init(ID3D12Device* device)
{
	m_device = device;

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	CreateCommandAllocatorAndList();
	CreateFence();
}

void D3D12CommandList::Clean()
{
	WaitForGpu();

	if (m_fenceEvent)
	{
		::CloseHandle(m_fenceEvent);
		m_fenceEvent = nullptr;
	}

	if (m_fence)
	{
		m_fence->Release();
		m_fence = nullptr;
	}

	if (m_commandList)
	{
		m_commandList->Release();
		m_commandList = nullptr;
	}

	if (m_commandAllocator)
	{
		m_commandAllocator->Release();
		m_commandAllocator = nullptr;
	}

	if (m_commandQueue)
	{
		m_commandQueue->Release();
		m_commandQueue = nullptr;
	}
}

void D3D12CommandList::Reset()
{
	ThrowIfFailed(m_commandAllocator->Reset());

	ThrowIfFailed(m_commandList->Reset(m_commandAllocator, nullptr));
}

void D3D12CommandList::ExcuteCommandLists()
{
	ThrowIfFailed(m_commandList->Close());

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_commandList };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

void D3D12CommandList::WaitForGpu()
{
	uint64 curFenceValue = m_fenceValue++;
	m_commandQueue->Signal(m_fence, curFenceValue);

	if (m_fence->GetCompletedValue() < curFenceValue)
	{
		m_fence->SetEventOnCompletion(curFenceValue, m_fenceEvent);
		::WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}

void D3D12CommandList::CreateCommandAllocatorAndList()
{
	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator, nullptr, IID_PPV_ARGS(&m_commandList)));

	ThrowIfFailed(m_commandList->Close());
}

void D3D12CommandList::CreateFence()
{
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceValue = 1;

	// Create an event handle to use for frame synchronization.
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
}
