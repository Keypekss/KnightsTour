#include "SceneRenderer.h"
#include <DirectXColors.h>


using namespace DirectX;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		SceneRenderer theApp(hInstance);
		if (!theApp.Initialize())
			return 0;

		return theApp.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}

SceneRenderer::SceneRenderer(HINSTANCE hInstance)
	: DXApp(hInstance)
{
}

SceneRenderer::~SceneRenderer()
{
}

bool SceneRenderer::Initialize()
{
	if (!DXApp::Initialize())
		return false;

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	BuildBuffers();
	BuildConstantBuffer();
	LoadTexture();
	BuildDescriptorHeaps();
	BuildRootSignature();
	BuildShadersAndInputLayout();	
	BuildPSO();
	LoadTiles();

	// Execute the initialization commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	return true;
}

void SceneRenderer::OnResize()
{
	DXApp::OnResize();
}

void SceneRenderer::OnUpdate(const Timer& gt)
{
	
}

void SceneRenderer::OnMouseDown(WPARAM btnState, int x, int y)
{
	int index = ScreenCoordToIndex(x, y);
	if (KnightsTour::enforce_next_move(index)) {
		KnightsTour::set_current_move_iterator(index);
		KnightsTour::calculate_visitable_tile(KnightsTour::currentMoveItr);
		KnightsTour::make_move(KnightsTour::currentMoveItr);
		LoadTiles();
	}

	if (KnightsTour::visitableTileExists == false) {
		std::wstring controls = L"Nowhere to move from here.\n"
			"Press U to undo your actions.\n"
			"Press C to start over.";
		MessageBox(nullptr, controls.c_str(), L"Controls", MB_OK);
	}
}

void SceneRenderer::OnKeyUp(WPARAM button) {
	switch (button)
	{
	case VK_ESCAPE:
		PostQuitMessage(0);
		break;
	case 0x43: // 'C' button
		KnightsTour::clear_screen();
		break;
	case 0x55: // 'U' button
		KnightsTour::undo_move();
		break;
	case 0x52: // 'R' button
		KnightsTour::redo_move();
		break;
	default:
		break;
	}

	if (KnightsTour::movesMade.empty() == false) {
		if (KnightsTour::enforce_next_move(*KnightsTour::currentMoveItr)) {
			KnightsTour::calculate_visitable_tile(KnightsTour::currentMoveItr);
			KnightsTour::make_move(KnightsTour::currentMoveItr);
		}
	}

	LoadTiles();
}

void SceneRenderer::Draw(const Timer& gt)
{
	ThrowIfFailed(mDirectCmdListAlloc->Reset());
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPipelineStateObject.Get()));

	// populate the command list
	{
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		mCommandList->RSSetViewports(1, &mScreenViewport);
		mCommandList->RSSetScissorRects(1, &mScissorRect);

		mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::Black, 0, nullptr);

		mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

		mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

		mCommandList->IASetVertexBuffers(0, 1, &mVertexBufferView);
		mCommandList->IASetIndexBuffer(&mIndexBufferView);		

		mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// set srv heap
		ID3D12DescriptorHeap* srvDescriptorHeaps[] = { mSrvHeap.Get() };
		mCommandList->SetDescriptorHeaps(_countof(srvDescriptorHeaps), srvDescriptorHeaps);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
		mCommandList->SetGraphicsRootDescriptorTable(1, tex);

// 			LoadTilePositions(tile);
		for(int tile = 0; tile < KnightsTour::chessboard.size(); ++tile) {
			mCommandList->SetGraphicsRootConstantBufferView(0, mConstantBuffer->GetGPUVirtualAddress() + (sizeof(mConstantBufferData) * tile));
			mCommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
		}

		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	}

	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Wait until frame commands are complete.  This waiting is inefficient and is done for simplicity. 
	FlushCommandQueue();
}

void SceneRenderer::BuildRootSignature()
{
	CD3DX12_ROOT_PARAMETER1 rootParameters[2] = {};
	CD3DX12_DESCRIPTOR_RANGE1 cbvTable = {}; 
	CD3DX12_DESCRIPTOR_RANGE1 srvTable = {}; 

	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	srvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

	rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE,  D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[1].InitAsDescriptorTable(1, &srvTable, D3D12_SHADER_VISIBILITY_PIXEL);	

	// sampler for texture
	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER); // addressW

	// create the root signature
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &pointClamp, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedSignature = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSignatureDesc, serializedSignature.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr) 
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	
	ThrowIfFailed(hr);

	ThrowIfFailed(mDevice->CreateRootSignature(0, serializedSignature->GetBufferPointer(), serializedSignature->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));
}

void SceneRenderer::BuildShadersAndInputLayout()
{
	mVertexShaderByteCode	= DXUtil::CompileShader(L"Shaders/Shader.hlsl", nullptr, "VS", "vs_5_1");
	mPixelShaderByteCode	= DXUtil::CompileShader(L"Shaders/Shader.hlsl", nullptr, "PS", "ps_5_1");

	mInputLayout = {
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void SceneRenderer::BuildBuffers()
{
	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT2 texCoord;
	};

	Vertex vertices[] = {
		// positions				// texCoords
		{ { -0.125f, -0.125f , 0.0f}, { 0.0f, 1.0f } },
		{ {  0.125f, -0.125f , 0.0f}, { 1.0f, 1.0f } },
		{ { -0.125f,  0.125f , 0.0f}, { 0.0f, 0.0f } },
		{ {  0.125f,  0.125f , 0.0f}, { 1.0f, 0.0f } }
	};

	// set indices
	uint16_t indices[] = {
		0, 2, 1,
		1, 2, 3
	};

	// sizes of buffers in terms of bytes
	const UINT vbByteSize = sizeof(vertices);
	const UINT ibByteSize = sizeof(indices);

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &mVertexBufferCPU));
	CopyMemory(mVertexBufferCPU->GetBufferPointer(), &vertices, vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mIndexBufferCPU));
	CopyMemory(mIndexBufferCPU->GetBufferPointer(), &indices, ibByteSize);

	// send buffers to the gpu
	mVertexBufferGPU = DXUtil::CreateDefaultBuffer(mDevice.Get(),
		mCommandList.Get(), &vertices, vbByteSize, mVertexBufferUploader);

	mIndexBufferGPU = DXUtil::CreateDefaultBuffer(mDevice.Get(),
		mCommandList.Get(), &indices, ibByteSize, mIndexBufferUploader);

	// set vertex buffer view
	mVertexBufferView.BufferLocation = mVertexBufferGPU->GetGPUVirtualAddress();
	mVertexBufferView.StrideInBytes = sizeof(Vertex);
	mVertexBufferView.SizeInBytes = vbByteSize;

	// set index buffer view
	mIndexBufferView.BufferLocation = mIndexBufferGPU->GetGPUVirtualAddress();
	mIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
	mIndexBufferView.SizeInBytes = ibByteSize;
}

void SceneRenderer::BuildConstantBuffer()
{
	// describe constant buffer heap

	const UINT	constantBufferSize = sizeof(TileConstantBuffer);
	
	// create constant buffer
	ThrowIfFailed(mDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mConstantBuffer)));

	ZeroMemory(&mConstantBufferData, sizeof(mConstantBufferData));

	// map and initialize constant buffer. don't unmap until the app closes.
	CD3DX12_RANGE readRange(0, 0); // we don't intend to read from this resource on the CPU.
	ThrowIfFailed(mConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mCbvDataBegin)));
	for(int i = 0; i < KnightsTour::chessboard.size(); ++i) {
		memcpy(mCbvDataBegin + (sizeof(mConstantBufferData) * i), &mConstantBufferData, sizeof(mConstantBufferData));
	}
}

void SceneRenderer::BuildDescriptorHeaps()
{
	// create srv heap
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(mDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvHeap)));

	// fill the heap with descriptors
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvHeap->GetCPUDescriptorHandleForHeapStart());

	auto dvdTexture = mTileTexture->Resource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = dvdTexture->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = dvdTexture->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	mDevice->CreateShaderResourceView(dvdTexture.Get(), &srvDesc, hDescriptor);
}

void SceneRenderer::LoadTexture()
{
	mTileTexture = std::make_unique<Texture>();
	mTileTexture->Filename = L"Textures/tile.dds";
	mTileTexture->Name = "tileTexture";

	ResourceUploadBatch resourceUpload(mDevice.Get());
	resourceUpload.Begin();
	
	ThrowIfFailed(CreateDDSTextureFromFile(mDevice.Get(), resourceUpload, (mTileTexture->Filename).c_str(), mTileTexture->Resource.ReleaseAndGetAddressOf()));

	// upload resources to the gpu
	auto uploadResourcesFinished = resourceUpload.End(mCommandQueue.Get());

	uploadResourcesFinished.wait();

}

void SceneRenderer::UpdateMVP() {

}

void SceneRenderer::ShowControls()
{
	std::wstring controls = L"Controls:\n"
		"Press U to undo move\n"
		"Press R to redo move\n"
		"Press C to clear screen\n";
	MessageBox(nullptr, controls.c_str(), L"Controls", MB_OK);
}

void SceneRenderer::LoadTiles()
{

	// build projection matrix
	XMMATRIX tmpMat = XMMatrixOrthographicLH(2, 2, 0.0f, 1.0f);
	XMStoreFloat4x4(&projectionMatrix, tmpMat);

	XMFLOAT4 cameraPosition { 0.0f, 0.0f, -1.0f, 0.0f };
	XMFLOAT4 cameraTarget { 0.0f, 0.0f, 0.0f, 0.0f };
	XMFLOAT4 cameraUp { 0.0f, 1.0f, 0.0f, 0.0f };

	// build view matrix
	XMVECTOR cPos = XMLoadFloat4(&cameraPosition);
	XMVECTOR cTarg = XMLoadFloat4(&cameraTarget);
	XMVECTOR cUp = XMLoadFloat4(&cameraUp);
	tmpMat = XMMatrixLookAtLH(cPos, cTarg, cUp);
	XMStoreFloat4x4(&viewMatrix, tmpMat);

	int bufferOffset = 0;
	DirectX::XMFLOAT4 positionOffset { -0.875, 0.875, 0, 0 };
	for(int row = rows - 1; row >= 0; --row) {
		for(uint8_t column = 0; column < columns; ++column) {
			Tile& tile = KnightsTour::chessboard.at(((rows * row) + column));
			tile.position.x = positionOffset.x;
			tile.position.y = positionOffset.y;
			tile.position.z = positionOffset.z;
			
			XMVECTOR posVec = XMLoadFloat4(&tile.position); // create xmvector for tile position
			tmpMat = XMMatrixTranslationFromVector(posVec); // create translation matrix from cube1's position vector
			XMStoreFloat4x4(&tile.worldMatrix, XMMatrixIdentity()); // initialize cube1's rotation matrix to identity matrix
			XMStoreFloat4x4(&tile.worldMatrix, XMMatrixTranspose(tmpMat)); // store cube1's world matrix

			XMMATRIX viewMat = XMLoadFloat4x4(&viewMatrix); // load view matrix
			XMMATRIX projMat = XMLoadFloat4x4(&projectionMatrix); // load projection matrix
			XMMATRIX wvpMat = XMLoadFloat4x4(&tile.worldMatrix) * viewMat * projMat; // create wvp matrix
			XMMATRIX transposed = XMMatrixTranspose(wvpMat);
			XMStoreFloat4x4(&mConstantBufferData.mvp, wvpMat);
			mConstantBufferData.color = tile.color;
			
			memcpy(mCbvDataBegin + (sizeof(mConstantBufferData) * bufferOffset), &mConstantBufferData, sizeof(mConstantBufferData));

			positionOffset.x += 0.25f;
			++bufferOffset;
		}
		positionOffset.y -= 0.25f;
		positionOffset.x = -0.875f;
	}
}

int SceneRenderer::ScreenCoordToIndex(int x, int y)
{
	// Screen coordinates are 0,0 at top left where 0,0 is at the bottom left on the chessboard.
	// Do the necessary mapping. (newX = width - oldY, newY = oldX)
	constexpr int tileLengthInPixels = 100;
	int row = (mWidth - y) / tileLengthInPixels;
	int column = x / tileLengthInPixels;

	std::wstring index = std::to_wstring(row * rows + column);
	index += L"\n";
	OutputDebugString(index.c_str());

	return row * rows + column;
}

void SceneRenderer::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = mRootSignature.Get();
	psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(mVertexShaderByteCode.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(mPixelShaderByteCode.Get());
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = mBackBufferFormat;
	psoDesc.DSVFormat = mDepthStencilFormat;
	psoDesc.SampleDesc = { 1, 0 };

	ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPipelineStateObject)));
}


