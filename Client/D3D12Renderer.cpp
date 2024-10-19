#include "pch.h"
#include "D3D12Renderer.h"
#include "D3D12Utils.h"

/*
==================
D3D12Renderer
==================
*/

bool D3D12Renderer::Init(HWND hwnd)
{
	m_hwnd = hwnd;

#if defined(_DEBUG)
	// Enable the D3D12 debug layer.
	{
		ID3D12Debug* debugController = nullptr;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			ID3D12Debug2* debugController2 = nullptr;
			if (SUCCEEDED(debugController->QueryInterface(IID_PPV_ARGS(&debugController2))))
			{
				debugController2->SetGPUBasedValidationFlags(D3D12_GPU_BASED_VALIDATION_FLAGS_NONE);
			}
		}
		debugController->Release();
		debugController = nullptr;
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
	swapChainDesc.BufferCount = s_FrameCount;
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

	CreateDescriptorHeap();
	CreateFrameResources();
	CreateCommandAllocatorAndList();
	CreateFence();

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

	return true;
}

void D3D12Renderer::Clean()
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

	for (uint32 i = 0; i < s_FrameCount; i++)
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
			IDXGIDebug1* debug = nullptr;
			if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug))))
			{
				debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
				debug->Release();
				debug = nullptr;
			}
		}
	}
}

void D3D12Renderer::Update()
{
}

void D3D12Renderer::BeginRender()
{
	ThrowIfFailed(m_commandAllocator->Reset());

	ThrowIfFailed(m_commandList->Reset(m_commandAllocator, nullptr));

	// Set necessary state.
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	// Indicate that the back buffer will be used as a render target.
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &barrier);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
}

void D3D12Renderer::EndRender()
{
	// Indicate that the back buffer will now be used to present.
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &barrier);

	ThrowIfFailed(m_commandList->Close());

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_commandList };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

void D3D12Renderer::Present()
{
	// Present the frame.
	ThrowIfFailed(m_swapChain->Present(1, 0));

	WaitForPreviousFrame();
}

D3D12Mesh* D3D12Renderer::CreateMesh(Vertex* vertices, uint32 verticesSize, uint32* indices, uint32 indicesSize)
{
	// Current, vertice = nullptr !!!
	// TODO

	D3D12Mesh* mesh = new D3D12Mesh;
	mesh->Init(m_device, vertices, verticesSize, indices, indicesSize);

	return mesh;
}

void D3D12Renderer::RenderMesh(D3D12Mesh* mesh)
{
	mesh->Render(m_commandList);
}

void D3D12Renderer::DestroyMesh(D3D12Mesh* mesh)
{
	if (mesh)
	{
		mesh->Clean();
		delete mesh;
		mesh = nullptr;
	}
}

void D3D12Renderer::CreateDescriptorHeap()
{
	// Describe and create a render target view (RTV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = s_FrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void D3D12Renderer::CreateFrameResources()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

	// Create a RTV for each frame.
	for (UINT n = 0; n < s_FrameCount; n++)
	{
		ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
		m_device->CreateRenderTargetView(m_renderTargets[n], nullptr, rtvHandle);
		rtvHandle.Offset(1, m_rtvDescriptorSize);
	}
}


void D3D12Renderer::CreateCommandAllocatorAndList()
{
	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator, nullptr, IID_PPV_ARGS(&m_commandList)));

	ThrowIfFailed(m_commandList->Close());
}

void D3D12Renderer::CreateFence()
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

void D3D12Renderer::WaitForPreviousFrame()
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

/*
================
D3D12Mesh
================
*/

bool D3D12Mesh::Init(ID3D12Device* device, Vertex* vertices, uint32 vertexSize, uint32* indices, uint32 indexSize)
{
	m_device = device;

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

	// Create the vertex buffer.
	m_vertexBuffer = D3D12Utils::CreateVertexBuffer(m_device, vertices, vertexSize, sizeof(Vertex));

	return true;
}

void D3D12Mesh::Clean()
{
	if (m_vertexBuffer)
	{
		m_vertexBuffer->resource->Release();
		m_vertexBuffer->resource = nullptr;
		
		delete m_vertexBuffer;
		m_vertexBuffer = nullptr;
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
}

void D3D12Mesh::Update()
{
}

void D3D12Mesh::Render(ID3D12GraphicsCommandList* commandList)
{
	commandList->SetGraphicsRootSignature(m_rootSignature);
	commandList->SetPipelineState(m_pipelineState);
	// Record commands.
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &m_vertexBuffer->vertexBufferView);
	commandList->DrawInstanced(3, 1, 0, 0);
}
