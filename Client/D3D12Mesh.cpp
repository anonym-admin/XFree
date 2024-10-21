#include "pch.h"
#include "D3D12Mesh.h"
#include "D3D12Utils.h"
#include "D3D12Renderer.h"

/*
================
D3D12Mesh.
================
*/

uint32 D3D12Mesh::sm_refCount = 0;
ID3D12RootSignature* D3D12Mesh::sm_rootSignature = nullptr;
ID3D12PipelineState* D3D12Mesh::sm_pipelineState = nullptr;

bool D3D12Mesh::Init(D3D12Renderer* renderer, MeshData meshData)
{
	m_renderer = renderer;
	m_meshData = meshData;

	ID3D12Device* device = m_renderer->GetDevice();

	if (sm_refCount == 0)
	{
		CreateRootSignature();
		CreatePipelineState();
	}

	// Create buffers.
	m_vertexBuffer = D3D12Utils::CreateVertexBuffer(device, meshData.vertices, meshData.verticesCount, meshData.verticesSize, sizeof(Vertex));
	m_indexBuffer = D3D12Utils::CreateIndexBuffer(device, meshData.indices, meshData.indicesCount, meshData.indicesSize, DXGI_FORMAT_R32_UINT);

	// Create const buffer.
	CreateDesciptorHeap();
	CreateConstantBuffer();

	sm_refCount++;

	return true;
}

void D3D12Mesh::Clean()
{
	uint32 refCount = --sm_refCount;
	if (refCount != 0)
	{
		return;
	}

	DestroyConstantBuffer();
	DestroyDescriptorHeap();

	if (m_indexBuffer)
	{
		m_indexBuffer->resource->Release();
		m_indexBuffer->resource = nullptr;

		delete m_indexBuffer;
		m_indexBuffer = nullptr;
	}

	if (m_vertexBuffer)
	{
		m_vertexBuffer->resource->Release();
		m_vertexBuffer->resource = nullptr;

		delete m_vertexBuffer;
		m_vertexBuffer = nullptr;
	}

	DestroyPipelineState();
	DestroyRootSignature();

	// TODO
	if (m_meshData.vertices)
	{
		delete[] m_meshData.vertices;
		m_meshData.vertices = nullptr;
	}

	if (m_meshData.indices)
	{
		delete[] m_meshData.indices;
		m_meshData.indices = nullptr;
	}
}

void D3D12Mesh::UpdateWorldMatrix(Matrix worldRow)
{
	m_worldRow = worldRow;
}

void D3D12Mesh::Update()
{
	m_constData.world = m_worldRow;
	m_constData.world = m_constData.world.Transpose();
	m_constData.view = DirectX::XMMatrixLookToLH(Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f));
	m_constData.view = m_constData.view.Transpose();
	m_constData.proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(70.0f), m_renderer->GetAspectRatio(), 0.1f, 100.0f);
	m_constData.proj = m_constData.proj.Transpose();

	uint32 bufferSize = D3D12Utils::CalcConstantBufferByteSize(sizeof(ConstBufferData));

	::memcpy(m_mappedData, &m_constData, bufferSize);
}

void D3D12Mesh::Render(ID3D12GraphicsCommandList* commandList)
{
	commandList->SetGraphicsRootSignature(sm_rootSignature);
	commandList->SetPipelineState(sm_pipelineState);
	commandList->SetDescriptorHeaps(1, &m_cbvHeap);
	commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
	// Record commands.
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &m_vertexBuffer->vertexBufferView);
	commandList->IASetIndexBuffer(&m_indexBuffer->indexBufferView);
	commandList->DrawIndexedInstanced(m_meshData.indicesCount, 1, 0, 0, 0);
}

void D3D12Mesh::CreateRootSignature()
{
	ID3D12Device* device = m_renderer->GetDevice();

	// Create a single descriptor table of CBVs.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(slotRootParameter), slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* signature = nullptr;
	ID3DBlob* error = nullptr;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&sm_rootSignature)));

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

void D3D12Mesh::CreatePipelineState()
{
	ID3D12Device* device = m_renderer->GetDevice();

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
	psoDesc.pRootSignature = sm_rootSignature;
	psoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.SampleDesc.Count = 1;
	ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&sm_pipelineState)));

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

void D3D12Mesh::CreateDesciptorHeap()
{
	ID3D12Device* device = m_renderer->GetDevice();

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));

	m_cbvsrvDesciptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void D3D12Mesh::CreateConstantBuffer()
{
	ID3D12Device* device = m_renderer->GetDevice();

	uint32 bufferCount = 1;
	uint32 bufferSize = D3D12Utils::CalcConstantBufferByteSize(sizeof(ConstBufferData));

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize * bufferCount),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_constBuffer)));

	ThrowIfFailed(m_constBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData)));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_constBuffer->GetGPUVirtualAddress();
	// Offset to the ith object constant buffer in the buffer.
	int32 boxCBufIndex = 0;
	cbAddress += boxCBufIndex * bufferSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = bufferSize;

	device->CreateConstantBufferView(
		&cbvDesc,
		m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
}

void D3D12Mesh::DestroyRootSignature()
{
	if (sm_rootSignature)
	{
		sm_rootSignature->Release();
		sm_rootSignature = nullptr;
	}
}

void D3D12Mesh::DestroyPipelineState()
{
	if (sm_pipelineState)
	{
		sm_pipelineState->Release();
		sm_pipelineState = nullptr;
	}
}

void D3D12Mesh::DestroyDescriptorHeap()
{
	if (m_cbvHeap)
	{
		m_cbvHeap->Release();
		m_cbvHeap = nullptr;
	}
}

void D3D12Mesh::DestroyConstantBuffer()
{
	if (m_constBuffer)
	{
		m_constBuffer->Unmap(0, nullptr);

		m_constBuffer->Release();
		m_constBuffer = nullptr;
	}
}
