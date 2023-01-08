#pragma once
#include "DXApp.h"
#include "KnightsTour.h"

using Microsoft::WRL::ComPtr;

struct TileConstantBuffer {
	DirectX::XMFLOAT4X4 mvp;
	DirectX::XMFLOAT4 color;
	float padding[44]; // constant buffer size must be multiple of 256byte
};

struct Texture
{
	// Unique material name for lookup.
	std::string Name;

	std::wstring Filename;

	ComPtr<ID3D12Resource> Resource = nullptr;
	ComPtr<ID3D12Resource> UploadHeap = nullptr;
};

class SceneRenderer : public DXApp
{
public:
	SceneRenderer(HINSTANCE hInstance);
	~SceneRenderer();

	bool Initialize() override;

private:
	void OnResize() override;
	void OnUpdate(const Timer& gt) override;
	void OnMouseDown(WPARAM btnState, int x, int y) override;
	void OnKeyUp(WPARAM button) override;


	void Draw(const Timer& gt) override;
	void BuildPSO();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildBuffers();
	void BuildConstantBuffer();
	void BuildDescriptorHeaps();
	void LoadTexture();
	void UpdateMVP();

	void ShowControls();
	void LoadTiles();
	int ScreenCoordToIndex(int x, int y);
	
	// constant buffer
	ComPtr<ID3D12DescriptorHeap> mCbvHeap;
	ComPtr<ID3D12Resource> mConstantBuffer;
	UINT8* mCbvDataBegin = nullptr;
	TileConstantBuffer mConstantBufferData{};

	// dx necessary state
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	ComPtr<ID3DBlob> mVertexShaderByteCode = nullptr;
	ComPtr<ID3DBlob> mPixelShaderByteCode = nullptr;
	ComPtr<ID3D12RootSignature> mRootSignature;
	ComPtr<ID3D12PipelineState> mPipelineStateObject;

	// vertex and index buffers
	ComPtr<ID3DBlob> mVertexBufferCPU = nullptr;
	ComPtr<ID3DBlob> mIndexBufferCPU = nullptr;
	ComPtr<ID3D12Resource> mVertexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> mIndexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> mVertexBufferUploader = nullptr;
	ComPtr<ID3D12Resource> mIndexBufferUploader = nullptr;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView;

	// texture related
	std::unique_ptr<Texture> mTileTexture;
	ComPtr<ID3D12DescriptorHeap> mSrvHeap; 

	DirectX::XMFLOAT4X4 projectionMatrix; // this will store our projection matrix
	DirectX::XMFLOAT4X4 viewMatrix; // this will store our view matrix

};