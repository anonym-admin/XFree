#include "pch.h"
#include "D3D12Mesh.h"
#include "D3D12Utils.h"

/*
================
D3D12Mesh
================
*/

bool D3D12Mesh::Init(ID3D12Device* device, MeshData meshData)
{
	m_device = device;

	// Create an empty root signature.
	{
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
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
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

	// Create buffers.
	m_vertexBuffer = D3D12Utils::CreateVertexBuffer(m_device, meshData.vertices, meshData.verticesCount, meshData.verticesSize, sizeof(Vertex));
	m_indexBuffer = D3D12Utils::CreateIndexBuffer(m_device, meshData.indices, meshData.indicesCount, meshData.indicesSize, DXGI_FORMAT_R32_UINT);

	// Create const buffer.
	CreateDesciptorHeap();
	CreateConstantBuffer();

	return true;
}

void D3D12Mesh::Clean()
{
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
	m_constData.world = Matrix();
	m_constData.view = DirectX::XMMatrixLookToLH(Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f));
	m_constData.proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(70.0f), 1.7f, 0.1f, 100.0f);

	::memcpy(m_mappedData, &m_constData, sizeof(ConstBufferData));
}

void D3D12Mesh::Render(ID3D12GraphicsCommandList* commandList)
{
	commandList->SetGraphicsRootSignature(m_rootSignature);
	commandList->SetPipelineState(m_pipelineState);
	commandList->SetDescriptorHeaps(1, &m_cbvHeap);
	commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
	// Record commands.
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &m_vertexBuffer->vertexBufferView);
	commandList->DrawInstanced(3, 1, 0, 0);
}

void D3D12Mesh::CreateDesciptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));

	m_cbvsrvDesciptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void D3D12Mesh::CreateConstantBuffer()
{
	uint32 bufferCount = 1;
	uint32 bufferSize = D3D12Utils::CalcConstantBufferByteSize(sizeof(ConstBufferData));
	ThrowIfFailed(m_device->CreateCommittedResource(
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

	m_device->CreateConstantBufferView(
		&cbvDesc,
		m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
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
