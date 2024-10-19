#include "pch.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#pragma warning(disable : 6001)

#include <d3d12.h>
#include <dxgi1_6.h>
#include "d3dx12.h"
#include <dxgidebug.h>
#include <d3dcompiler.h>
#include <directxtk/SimpleMath.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		int32 errorCode = hr;
		__debugbreak();
	}
}

using DirectX::SimpleMath::Vector4;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector2;

/*
=======
Vertex
=======
*/

struct Vertex
{
	Vector3 posModel = {};
	Vector4 color = {};
};

/*
=======
InGame
=======
*/

class InGame
{
public:
	bool Init(HWND hwnd)
	{
		m_hwnd = hwnd;

		LoadPipelines();
		LoadAssets();

		return true;
	}

	void Clean()
	{
		WaitForPreviousFrame();

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

		if (m_vertexBuffer)
		{
			m_vertexBuffer->Release();
			m_vertexBuffer = nullptr;
		}

		if (m_commandList)
		{
			m_commandList->Release();
			m_commandList = nullptr;
		}

		if (m_pipelineState)
		{
			m_pipelineState->Release();
			m_pipelineState = nullptr;
		}

		if (m_rootSignature)
		{
			m_rootSignature->Release();
			m_rootSignature = nullptr;
		}

		if (m_commandAllocator)
		{
			m_commandAllocator->Release();
			m_commandAllocator = nullptr;
		}

		for (uint32 i = 0; i < FrameCount; i++)
		{
			if (m_renderTargets[i])
			{
				m_renderTargets[i]->Release();
				m_renderTargets[i] = nullptr;
			}
		}

		if (m_rtvHeap)
		{
			m_rtvHeap->Release();
			m_rtvHeap = nullptr;
		}

		if (m_swapChain)
		{
			m_swapChain->Release();
			m_swapChain = nullptr;
		}

		if (m_commandQueue)
		{
			m_commandQueue->Release();
			m_commandQueue = nullptr;
		}

		if (m_device)
		{
			int32 refCount = m_device->Release();
			if (refCount != 0)
			{
				// TODO
				int32 test = 0;
			}
		}
	}

	void Update()
	{
	}

	void Render()
	{
		ThrowIfFailed(m_commandAllocator->Reset());

		ThrowIfFailed(m_commandList->Reset(m_commandAllocator, m_pipelineState));

		// Set necessary state.
		m_commandList->SetGraphicsRootSignature(m_rootSignature);
		m_commandList->RSSetViewports(1, &m_viewport);
		m_commandList->RSSetScissorRects(1, &m_scissorRect);

		// Indicate that the back buffer will be used as a render target.
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_commandList->ResourceBarrier(1, &barrier);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
		m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		// Record commands.
		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
		m_commandList->DrawInstanced(3, 1, 0, 0);

		// Indicate that the back buffer will now be used to present.
		barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		m_commandList->ResourceBarrier(1, &barrier);

		ThrowIfFailed(m_commandList->Close());

		// Execute the command list.
		ID3D12CommandList* ppCommandLists[] = { m_commandList };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// Present the frame.
		ThrowIfFailed(m_swapChain->Present(1, 0));

		WaitForPreviousFrame();
	}

private:
	const static uint32 FrameCount = 2;

	// Pipeline objects.
	D3D12_VIEWPORT m_viewport = {};
	D3D12_RECT m_scissorRect = {};
	IDXGISwapChain3* m_swapChain = nullptr;
	ID3D12Device* m_device = nullptr;
	ID3D12Resource* m_renderTargets[FrameCount] = {};
	ID3D12CommandAllocator* m_commandAllocator = nullptr;
	ID3D12CommandQueue* m_commandQueue = nullptr;
	ID3D12RootSignature* m_rootSignature = nullptr;
	ID3D12DescriptorHeap* m_rtvHeap = nullptr;
	ID3D12PipelineState* m_pipelineState = nullptr;
	ID3D12GraphicsCommandList* m_commandList = nullptr;
	uint32 m_rtvDescriptorSize = 0;

	// App resources.
	ID3D12Resource* m_vertexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};

	// Synchronization objects.
	uint32 m_frameIndex = 0;
	HANDLE m_fenceEvent = nullptr;
	ID3D12Fence* m_fence = nullptr;
	uint64 m_fenceValue = 0;

	bool m_useWarpDevice = true;
	HWND m_hwnd = nullptr;

	float m_aspectRatio = 0.0f;

	void LoadPipelines()
	{
#if defined(_DEBUG)
		// Enable the D3D12 debug layer.
		{
			ID3D12Debug* debugController = nullptr;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();
			}
			debugController->Release();
			debugController = nullptr;

			IDXGIDebug1* debugController2 = nullptr;
			if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debugController2))))
			{
				ThrowIfFailed(debugController2->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL));
			}
			//debugController2->Release();
			//debugController2 = nullptr;
		}
#endif

		IDXGIFactory4* factory = nullptr;
		ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

		if (m_useWarpDevice)
		{
			IDXGIAdapter* warpAdapter = nullptr;
			ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

			ThrowIfFailed(D3D12CreateDevice(warpAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));

			warpAdapter->Release();
			warpAdapter = nullptr;
		}
		else
		{
			IDXGIAdapter1* hardwareAdapter = nullptr;
			// GetHardwareAdapter(factory.Get(), &hardwareAdapter);

			ThrowIfFailed(D3D12CreateDevice(hardwareAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));

			hardwareAdapter->Release();
			hardwareAdapter = nullptr;
		}

		// Describe and create the command queue.
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

		RECT rt = {};
		::GetClientRect(m_hwnd, &rt);

		uint32 screenWidth = static_cast<uint32>(rt.right - rt.left);
		uint32 screenHeight = static_cast<uint32>(rt.bottom - rt.top);

		m_aspectRatio = static_cast<float>(screenWidth) / screenHeight;

		// Describe and create the swap chain.
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferCount = FrameCount;
		swapChainDesc.BufferDesc.Width = screenWidth;
		swapChainDesc.BufferDesc.Height = screenHeight;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.OutputWindow = m_hwnd;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.Windowed = TRUE;

		IDXGISwapChain* swapChain = nullptr;
		ThrowIfFailed(factory->CreateSwapChain(
			m_commandQueue,        // Swap chain needs the queue so that it can force a flush on it.
			&swapChainDesc,
			&swapChain
		));

		ThrowIfFailed(swapChain->QueryInterface(IID_PPV_ARGS(&m_swapChain)));
		swapChain->Release();

		// This sample does not support fullscreen transitions.
		ThrowIfFailed(factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		factory->Release();
		factory = nullptr;

		// Create descriptor heaps.
		{
			// Describe and create a render target view (RTV) descriptor heap.
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.NumDescriptors = FrameCount;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

			m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}

		// Create frame resources.
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

			// Create a RTV for each frame.
			for (UINT n = 0; n < FrameCount; n++)
			{
				ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
				m_device->CreateRenderTargetView(m_renderTargets[n], nullptr, rtvHandle);
				rtvHandle.Offset(1, m_rtvDescriptorSize);
			}
		}

		ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

		m_viewport.TopLeftX = 0.0f;
		m_viewport.TopLeftY = 0.0f;
		m_viewport.Width = static_cast<float>(screenWidth);
		m_viewport.Height = static_cast<float>(screenHeight);
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;

		m_scissorRect.left = 0;
		m_scissorRect.top = 0;
		m_scissorRect.right = screenWidth;
		m_scissorRect.bottom = screenHeight;
	}

	void LoadAssets()
	{
		// Create an empty root signature.
		{
			CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
			rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

			ID3DBlob* signature = nullptr;
			ID3DBlob* error = nullptr;
			ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
			ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));

			if (signature)
			{
				signature->Release();
				signature = nullptr;
			}
			if (error)
			{
				error->Release();
				error = nullptr;
			}
		}

		// Create the pipeline state, which includes compiling and loading shaders.
		{
			ID3DBlob* vertexShader = nullptr;
			ID3DBlob* pixelShader = nullptr;

#if defined(_DEBUG)
			// Enable better shader debugging with the graphics debugging tools.
			UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
			UINT compileFlags = 0;
#endif
			ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
			ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

			// Define the vertex input layout.
			D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};

			// Describe and create the graphics pipeline state object (PSO).
			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
			psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
			psoDesc.pRootSignature = m_rootSignature;
			psoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
			psoDesc.PS = { reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
			psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			psoDesc.DepthStencilState.DepthEnable = FALSE;
			psoDesc.DepthStencilState.StencilEnable = FALSE;
			psoDesc.SampleMask = UINT_MAX;
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psoDesc.NumRenderTargets = 1;
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psoDesc.SampleDesc.Count = 1;
			ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
			
			if (vertexShader)
			{
				vertexShader->Release();
				vertexShader = nullptr;
			}

			if (pixelShader)
			{
				pixelShader->Release();
				pixelShader = nullptr;
			}
		}

		// Create the command list.
		ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator, m_pipelineState, IID_PPV_ARGS(&m_commandList)));

		// Command lists are created in the recording state, but there is nothing
		// to record yet. The main loop expects it to be closed, so close it now.
		ThrowIfFailed(m_commandList->Close());

		// Create the vertex buffer.
		{
			// Define the geometry for a triangle.
			Vertex triangleVertices[] =
			{
				{ Vector3(0.0f, 0.25f * m_aspectRatio, 1.0f), Vector4(1.0f, 0.0f, 0.0f, 1.0f)},
				{ Vector3{ 0.25f, -0.25f * m_aspectRatio, 1.0f }, Vector4{ 0.0f, 1.0f, 0.0f, 1.0f } },
				{ Vector3{ -0.25f, -0.25f * m_aspectRatio, 1.0f }, Vector4{ 0.0f, 0.0f, 1.0f, 1.0f } }
			};

			const UINT vertexBufferSize = sizeof(triangleVertices);

			// Note: using upload heaps to transfer static data like vert buffers is not 
			// recommended. Every time the GPU needs it, the upload heap will be marshalled 
			// over. Please read up on Default Heap usage. An upload heap is used here for 
			// code simplicity and because there are very few verts to actually transfer.
			CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
			auto desc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
			ThrowIfFailed(m_device->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_vertexBuffer)));

			// Copy the triangle data to the vertex buffer.
			UINT8* pVertexDataBegin;
			CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
			ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
			memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
			m_vertexBuffer->Unmap(0, nullptr);

			// Initialize the vertex buffer view.
			m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
			m_vertexBufferView.StrideInBytes = sizeof(Vertex);
			m_vertexBufferView.SizeInBytes = vertexBufferSize;
		}

		// Create synchronization objects and wait until assets have been uploaded to the GPU.
		{
			ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
			m_fenceValue = 1;

			// Create an event handle to use for frame synchronization.
			m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (m_fenceEvent == nullptr)
			{
				ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
			}

			// Wait for the command list to execute; we are reusing the same command 
			// list in our main loop but for now, we just want to wait for setup to 
			// complete before continuing.
			WaitForPreviousFrame();
		}
	}

	void WaitForPreviousFrame()
	{
		uint64 curFenceValue = m_fenceValue++;
		m_commandQueue->Signal(m_fence, curFenceValue);

		if (m_fence->GetCompletedValue() < curFenceValue)
		{
			m_fence->SetEventOnCompletion(curFenceValue, m_fenceEvent);
			::WaitForSingleObject(m_fenceEvent, INFINITE);
		}

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	}
};


/*
=================
Main entry point
=================
*/

int main(int argc, char* argv[])
{
	// Register the window class.
	const wchar_t CLASS_NAME[] = L"Windows Class";
	const wchar_t WINDOW_NAME[] = L"XFree Engine Demo_v.1.0";

	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = ::GetModuleHandle(nullptr);
	wc.lpszClassName = CLASS_NAME;
	::RegisterClass(&wc);

	// Create the window.
	HWND hwnd = ::CreateWindowEx(0, CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, wc.hInstance, NULL);

	if (hwnd == NULL)
	{
		return 0;
	}

	ShowWindow(hwnd, SW_SHOW);

	InGame* inGame = new InGame;
	if (!inGame->Init(hwnd))
		return -1;

	MSG msg = { };
	while (true)
	{
		if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}

			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			inGame->Update();
			inGame->Render();
		}
	}

	inGame->Clean();
	delete inGame;
	inGame = nullptr;

	::CloseWindow(hwnd);
	hwnd = nullptr;

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
	{
		int width = LOWORD(lParam);  // Macro to get the low-order word.
		int height = HIWORD(lParam); // Macro to get the high-order word.
	}
	break;
	case WM_DESTROY:
	{
		::PostQuitMessage(-1);
	}
	break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


