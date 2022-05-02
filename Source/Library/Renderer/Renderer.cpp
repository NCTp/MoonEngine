#include "Renderer/Renderer.h"

namespace library {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Renderer

	  Summary:  Constructor

	  Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
				 m_immediateContext, m_immediateContext1, m_swapChain,
				 m_swapChain1, m_renderTargetView, m_depthStencil,
				 m_depthStencilView, m_cbChangeOnResize, m_camera,
				 m_projection, m_renderables, m_vertexShaders,
				 m_pixelShaders].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	Renderer::Renderer() :
		m_driverType(D3D_DRIVER_TYPE_HARDWARE),
		m_featureLevel(D3D_FEATURE_LEVEL_11_1),

		m_d3dDevice(nullptr),
		m_d3dDevice1(nullptr),
		m_immediateContext(nullptr),
		m_immediateContext1(nullptr),
		m_swapChain(nullptr),
		m_swapChain1(nullptr),
		m_renderTargetView(nullptr),
		m_depthStencil(nullptr),
		m_depthStencilView(nullptr),
		m_cbChangeOnResize(),
		m_camera(XMVectorSet(0.0f, 0.0f, -3.0f, 0.0f)),
		m_projection(),
		m_renderables(std::unordered_map<std::wstring, std::shared_ptr<Renderable>>()),
		m_vertexShaders(std::unordered_map<std::wstring, std::shared_ptr<VertexShader>>()),
		m_pixelShaders(std::unordered_map<std::wstring, std::shared_ptr<PixelShader>>())
	{}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Initialize

	  Summary:  Creates Direct3D device and swap chain

	  Args:     HWND hWnd
				  Handle to the window

	  Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
				 m_d3dDevice1, m_immediateContext1, m_swapChain1,
				 m_swapChain, m_renderTargetView, m_cbChangeOnResize,
				 m_projection, m_cbLights, m_camera, m_vertexShaders,
				 m_pixelShaders, m_renderables].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::Initialize(_In_ HWND hWnd)
	{
		HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;

        UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_DRIVER_TYPE driverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
        UINT numDriverTypes = ARRAYSIZE(driverTypes);

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        UINT numFeatureLevels = ARRAYSIZE(featureLevels);

        for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
        {
            m_driverType = driverTypes[driverTypeIndex];
            hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

            if (hr == E_INVALIDARG)
            {
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
            }

            if (SUCCEEDED(hr))
                break;
        }
        if (FAILED(hr))
            return hr;


        ComPtr<IDXGIFactory1> dxgiFactory(nullptr);
        {
            ComPtr<IDXGIDevice> dxgiDevice(nullptr);

            hr = m_d3dDevice.As(&dxgiDevice);
            if (SUCCEEDED(hr))
            {
                ComPtr<IDXGIAdapter> adapter(nullptr);

                hr = dxgiDevice->GetAdapter(adapter.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = adapter->GetParent(__uuidof(IDXGIFactory1), (&dxgiFactory));
                }
            }
        }
        if (FAILED(hr))
            return hr;

        // Create swap chain
        ComPtr<IDXGIFactory2> dxgiFactory2(nullptr);
        hr = dxgiFactory.As(&dxgiFactory2);
        if (dxgiFactory2)
        {
            // DirectX 11.1 or later
            hr = m_d3dDevice.As(&m_d3dDevice1);
            if (SUCCEEDED(hr))
            {
                hr = m_immediateContext.As(&m_immediateContext1);
            }

            DXGI_SWAP_CHAIN_DESC1 descSwapChain =
            {
                .Width = width,
                .Height = height,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .SampleDesc =
                {
                    .Count = 1,
                    .Quality = 0
                },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1
            };

            hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &descSwapChain, nullptr, nullptr, m_swapChain1.GetAddressOf());
            if (SUCCEEDED(hr))
            {
                hr = m_swapChain1.As(&m_swapChain);
            }
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC descSwapChain =
            {
                .BufferDesc =
                {
                    .Width = width,
                    .Height = height,
                    .RefreshRate = {.Numerator = 60, .Denominator = 1 },
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                },
                .SampleDesc =
                {
                    .Count = 1,
                    . Quality = 0
                },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1,
                .OutputWindow = hWnd,
                .Windowed = TRUE
            };

            hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &descSwapChain, m_swapChain.GetAddressOf());
        }

        dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

        if (FAILED(hr))
            return hr;

        // Create a render target view
        ComPtr<ID3D11Texture2D> pBackBuffer(nullptr);

        hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (&pBackBuffer));
        if (FAILED(hr))
            return hr;

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        if (FAILED(hr))
            return hr;

        // Create depth stencil texture
        D3D11_TEXTURE2D_DESC descDepth =
        {
            .Width = width,
            .Height = height,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc =
            {
                .Count = 1,
                .Quality = 0,
            },
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0,
            .MiscFlags = 0

        };

        hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
        if (FAILED(hr))
            return hr;


        // Create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
        {
            .Format = descDepth.Format,
            .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
            .Texture2D =
            {
                .MipSlice = 0,
            }

        };

        hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
        if (FAILED(hr))
            return hr;

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

        // Setup the viewport
        D3D11_VIEWPORT vp =
        {
            .TopLeftX = 0,
            .TopLeftY = 0,
            .Width = (FLOAT)width,
            .Height = (FLOAT)height,
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f
        };

        m_immediateContext->RSSetViewports(1, &vp);



        // Initialize the view matrix
        XMVECTOR eye = XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
        XMVECTOR at = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        //m_view = XMMatrixLookAtLH(eye, at, up);

        // Initialize the projection matrix
        m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, width / (FLOAT)height, 0.01f, 100.0f);

        D3D11_BUFFER_DESC descCbuffer = {
            .ByteWidth = sizeof(CBChangeOnResize),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0,
            .MiscFlags = 0,
            .StructureByteStride = 0
        };

        // Create CBChangeOnResize constant buffer and set.
        CBChangeOnResize cbChangesOnResize;
        cbChangesOnResize.Projection = XMMatrixTranspose(m_projection);

        D3D11_SUBRESOURCE_DATA cData =
        {
            .pSysMem = &cbChangesOnResize,
            .SysMemPitch = 0,
            .SysMemSlicePitch = 0

        };

        hr = m_d3dDevice->CreateBuffer(&descCbuffer, &cData, &m_cbChangeOnResize);
        if (FAILED(hr))
            return hr;

        m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());

#pragma region CreateCBLightsAndSetB3
		D3D11_BUFFER_DESC cbLightsDesc = {
			.ByteWidth = sizeof(CBLights),
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
			.CPUAccessFlags = 0,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		CBLights cbLights = { };

		D3D11_SUBRESOURCE_DATA cbLightsData = {
			.pSysMem = &cbLights,
			.SysMemPitch = 0,
			.SysMemSlicePitch = 0
		};

		hr = m_d3dDevice->CreateBuffer(&cbLightsDesc, &cbLightsData, &m_cbLights);
		if (FAILED(hr)) return hr;

		m_immediateContext->VSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
		m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
#pragma endregion

#pragma region InitializeShadersAndRenderables
		for (auto& vs : m_vertexShaders)
		{
			hr = vs.second->Initialize(m_d3dDevice.Get());
			if (FAILED(hr)) return hr;
		}

		for (auto& ps : m_pixelShaders)
		{
			hr = ps.second->Initialize(m_d3dDevice.Get());
			if (FAILED(hr)) return hr;
		}

		for (auto& renderable : m_renderables)
		{
			hr = renderable.second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
			if (FAILED(hr)) return hr;
		}
#pragma endregion

		// Initialize Camera
		hr = m_camera.Initialize(m_d3dDevice.Get());
		if (FAILED(hr)) return hr;

		// Set primitive topology
		m_immediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::AddRenderable

	  Summary:  Add a renderable object

	  Args:     PCWSTR pszRenderableName
				  Key of the renderable object
				const std::shared_ptr<Renderable>& renderable
				  Shared pointer to the renderable object

	  Modifies: [m_renderables].

	  Returns:  HRESULT
				  Status code.
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::AddRenderable(_In_ PCWSTR pszRenderableName, _In_ const std::shared_ptr<Renderable>& renderable)
	{
		if (m_renderables.count(pszRenderableName) > 0) return E_FAIL;
		m_renderables.insert({ pszRenderableName, renderable });

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::AddPointLight

	  Summary:  Add a point light

	  Args:     size_t index
				  Index of the point light
				const std::shared_ptr<PointLight>& pointLight
				  Shared pointer to the point light object

	  Modifies: [m_aPointLights].

	  Returns:  HRESULT
				  Status code.
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::AddPointLight(_In_ size_t index, _In_ const std::shared_ptr<PointLight>& pPointLight)
	{
		if (index >= NUM_LIGHTS || !pPointLight)
		{
			return E_FAIL;
		}

		m_aPointLights[index] = pPointLight;
		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::AddVertexShader

	  Summary:  Add the vertex shader into the renderer

	  Args:     PCWSTR pszVertexShaderName
				  Key of the vertex shader
				const std::shared_ptr<VertexShader>&
				  Vertex shader to add

	  Modifies: [m_vertexShaders].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::AddVertexShader(_In_ PCWSTR pszVertexShaderName, _In_ const std::shared_ptr<VertexShader>& vertexShader)
	{
		if (m_vertexShaders.count(pszVertexShaderName) > 0) return E_FAIL;
		m_vertexShaders.insert({ pszVertexShaderName, vertexShader });

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::AddPixelShader

	  Summary:  Add the pixel shader into the renderer

	  Args:     PCWSTR pszPixelShaderName
				  Key of the pixel shader
				const std::shared_ptr<PixelShader>&
				  Pixel shader to add

	  Modifies: [m_pixelShaders].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::AddPixelShader(_In_ PCWSTR pszPixelShaderName, _In_ const std::shared_ptr<PixelShader>& pixelShader)
	{
		if (m_pixelShaders.count(pszPixelShaderName) > 0) return E_FAIL;
		m_pixelShaders.insert({ pszPixelShaderName, pixelShader });

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::HandleInput

	  Summary:  Add the pixel shader into the renderer and initialize it

	  Args:     const DirectionsInput& directions
				  Data structure containing keyboard input data
				const MouseRelativeMovement& mouseRelativeMovement
				  Data structure containing mouse relative input data

	  Modifies: [m_camera].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
	{
		m_camera.HandleInput(
			directions,
			mouseRelativeMovement,
			deltaTime
		);
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Update

	  Summary:  Update the renderables each frame

	  Args:     FLOAT deltaTime
				  Time difference of a frame
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderer::Update(_In_ FLOAT deltaTime)
	{
		for (auto renderablesElem : m_renderables)
		{
			renderablesElem.second->Update(deltaTime);

		}

		for (auto& light : m_aPointLights) 
		{
			light->Update(deltaTime);

		}

		m_camera.Update(deltaTime);

	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::Render

	  Summary:  Render the frame
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderer::Render()
	{
		// Clear the backbuffer
		const float ClearColor[4] = { 0.0f, 0.125f, 0.6f, 1.0f }; // RGBA
		m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), ClearColor);

		// Clear the depth buffer to 1.0 (maximum depth)
		m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// Create camera constant buffer and update
		XMFLOAT4 camPos;
		XMStoreFloat4(&camPos, m_camera.GetEye());
		CBChangeOnCameraMovement cbCamera = {
			.View = XMMatrixTranspose(m_camera.GetView()),
			.CameraPosition = camPos,
		};

		m_immediateContext->UpdateSubresource(
			m_camera.GetConstantBuffer().Get(),
			0u,
			nullptr,
			&cbCamera,
			0u,
			0u
		);
		m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
		m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());

		// Create light constant buffer and update
		CBLights cbLights = { };

		for (int i = 0; i < NUM_LIGHTS; i++)
		{
			cbLights.LightPositions[i] = m_aPointLights[i]->GetPosition();
			cbLights.LightColors[i] = m_aPointLights[i]->GetColor();
		}

		m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0, nullptr, &cbLights, 0, 0);

		m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

		// For each renderables
		for (auto& pair : m_renderables)
		{
			auto& renderable = pair.second;

			// Set the vertex buffer
			UINT stride = sizeof(SimpleVertex);
			UINT offset = 0;

			m_immediateContext->IASetVertexBuffers(
				0,												// the first input slot for binding
				1,												// the number of buffers in the array
				renderable->GetVertexBuffer().GetAddressOf(),	// the array of vertex buffers
				&stride,										// array of stride values, one for each buffer
				&offset
			);

			// Set the index buffer
			m_immediateContext->IASetIndexBuffer(renderable->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

			// Set the input layout
			m_immediateContext->IASetInputLayout(renderable->GetVertexLayout().Get());

			// Create and update renderable constant buffer
			CBChangesEveryFrame cbRenderable = {
				.World = XMMatrixTranspose(renderable->GetWorldMatrix()),
				.OutputColor = renderable->GetOutputColor()
			};


			m_immediateContext->UpdateSubresource(
				renderable->GetConstantBuffer().Get(),
				0u,
				nullptr,
				&cbRenderable,
				0u,
				0u
			);

			// Set shaders
			m_immediateContext->VSSetShader(renderable->GetVertexShader().Get(), nullptr, 0);
			m_immediateContext->PSSetShader(renderable->GetPixelShader().Get(), nullptr, 0);

			// Set renderable constant buffer
			m_immediateContext->VSSetConstantBuffers(2, 1, renderable->GetConstantBuffer().GetAddressOf());
			m_immediateContext->PSSetConstantBuffers(2, 1, renderable->GetConstantBuffer().GetAddressOf());

			if (renderable->HasTexture())
			{
				// Set texture resource view of the renderable into the pixel shader
				m_immediateContext->PSSetShaderResources(0, 1, renderable->GetTextureResourceView().GetAddressOf());

				// Set sampler state of the renderable into the pixel shader
				m_immediateContext->PSSetSamplers(0, 1, renderable->GetSamplerState().GetAddressOf());
			}

			// Render
			m_immediateContext->DrawIndexed(renderable->GetNumIndices(), 0, 0);
		}

		// Present
		m_swapChain->Present(0, 0);

	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::SetVertexShaderOfRenderable

	  Summary:  Sets the vertex shader for a renderable

	  Args:     PCWSTR pszRenderableName
				  Key of the renderable
				PCWSTR pszVertexShaderName
				  Key of the vertex shader

	  Modifies: [m_renderables].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::SetVertexShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszVertexShaderName)
	{

		if (!m_renderables.contains(pszRenderableName)) {
			return E_FAIL;

		}

		else {

			if (m_vertexShaders.contains(pszVertexShaderName)) {
				m_renderables.find(pszRenderableName)->second->SetVertexShader(m_vertexShaders.find(pszVertexShaderName)->second);
				return S_OK;

			}
			return E_FAIL;

		}

	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::SetPixelShaderOfRenderable

	  Summary:  Sets the pixel shader for a renderable

	  Args:     PCWSTR pszRenderableName
				  Key of the renderable
				PCWSTR pszPixelShaderName
				  Key of the pixel shader

	  Modifies: [m_renderables].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Renderer::SetPixelShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszPixelShaderName)
	{

		if (!m_renderables.contains(pszRenderableName)) {
			return E_FAIL;

		}

		else {

			if (m_vertexShaders.contains(pszPixelShaderName)) {
				m_renderables.find(pszRenderableName)->second->SetPixelShader(m_pixelShaders.find(pszPixelShaderName)->second);
				return S_OK;

			}

			return E_FAIL;

		}
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderer::GetDriverType

	  Summary:  Returns the Direct3D driver type

	  Returns:  D3D_DRIVER_TYPE
				  The Direct3D driver type used
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	D3D_DRIVER_TYPE Renderer::GetDriverType() const
	{
		return m_driverType;

	};
}







