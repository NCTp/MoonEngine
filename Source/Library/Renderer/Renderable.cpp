﻿#include "Renderer/Renderable.h"

#include "Texture/DDSTextureLoader.h"

namespace library
{
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::Renderable

	  Summary:  Constructor

	  Args:     const XMFLOAT4& outputColor
				  Default color of the renderable

	  Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer,
				 m_textureRV, m_samplerLinear, m_vertexShader,
				 m_pixelShader, m_textureFilePath, m_outputColor,
				 m_world].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	/*--------------------------------------------------------------------
	  TODO: Renderable::Renderable definition (remove the comment)
	--------------------------------------------------------------------*/
	Renderable::Renderable(_In_ const XMFLOAT4& outputColor) :

		m_vertexBuffer(nullptr),
		m_indexBuffer(nullptr),
		m_constantBuffer(nullptr),
		m_aMeshes(std::vector<BasicMeshEntry>()),
		m_aMaterials(std::vector<Material>()),
		m_vertexShader(nullptr),
		m_pixelShader(nullptr),
		m_outputColor(outputColor),
		m_padding(),
		m_world(XMMatrixIdentity())

	{};

	

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::initialize

	  Summary:  Initializes the buffers, texture, and the world matrix

	  Args:     ID3D11Device* pDevice
				  The Direct3D device to create the buffers
				ID3D11DeviceContext* pImmediateContext
				  The Direct3D context to set buffers

	  Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer].

	  Returns:  HRESULT
				  Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	/*--------------------------------------------------------------------
	  TODO: Renderable::initialize definition (remove the comment)
	--------------------------------------------------------------------*/
	HRESULT Renderable::initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext)
	{
		HRESULT hr = S_OK;

		// Create the vertex buffer
		D3D11_BUFFER_DESC bd =
		{
		.ByteWidth = sizeof(SimpleVertex) * GetNumVertices(),
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		.CPUAccessFlags = 0,

		};

		D3D11_SUBRESOURCE_DATA InitData =
		{
			.pSysMem = getVertices()

		};

		hr = pDevice->CreateBuffer(&bd, &InitData, m_vertexBuffer.GetAddressOf());
		if (FAILED(hr))
			return hr;

		// Create the index buffer
		D3D11_BUFFER_DESC iBufferDesc = 
		{
			.ByteWidth = static_cast<UINT>(sizeof(WORD)) * GetNumIndices(),
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_INDEX_BUFFER,
			.CPUAccessFlags = 0,

		};

		D3D11_SUBRESOURCE_DATA iData = 
		{
			.pSysMem = getIndices(),

		};

		hr = pDevice->CreateBuffer(&iBufferDesc, &iData, &m_indexBuffer);
		if (FAILED(hr)) return hr;
		
		// Create the constant buffer
		
		D3D11_BUFFER_DESC cBufferDesc = {
			.ByteWidth = sizeof(CBChangesEveryFrame),
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
			.CPUAccessFlags = 0,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};


		
		CBChangesEveryFrame cb = {
			.World = XMMatrixTranspose(m_world),
			//.OutputColor = m_outputColor
		};
		
		D3D11_SUBRESOURCE_DATA cData = {
			.pSysMem = &cb,
			.SysMemPitch = 0,
			.SysMemSlicePitch = 0
		};

		hr = pDevice->CreateBuffer(&cBufferDesc, &cData, &m_constantBuffer);
		if (FAILED(hr)) return hr;

		/*
		if (m_bHasTextures)
		{
			// Create texture resource device
			hr = CreateDDSTextureFromFile(
				pDevice,
				m_textureFilePath.filename().wstring().c_str(),
				nullptr,
				&m_textureRV
			);
			if (FAILED(hr)) return hr;

			// Create sampler state
			D3D11_SAMPLER_DESC sampDesc = {
				.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
				.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
				.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
				.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
				.ComparisonFunc = D3D11_COMPARISON_NEVER,
				.MinLOD = 0,
				.MaxLOD = D3D11_FLOAT32_MAX
			};
			hr = pDevice->CreateSamplerState(&sampDesc, &m_samplerLinear);
			if (FAILED(hr)) return hr;
		}

		*/
		return hr;

	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::SetVertexShader

	  Summary:  Sets the vertex shader to be used for this renderable
				object

	  Args:     const std::shared_ptr<VertexShader>& vertexShader
				  Vertex shader to set to

	  Modifies: [m_vertexShader].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderable::SetVertexShader(_In_ const std::shared_ptr<VertexShader>& vertexShader)
	{
		m_vertexShader = vertexShader;

	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::SetPixelShader

	  Summary:  Sets the pixel shader to be used for this renderable
				object

	  Args:     const std::shared_ptr<PixelShader>& pixelShader
				  Pixel shader to set to

	  Modifies: [m_pixelShader].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderable::SetPixelShader(_In_ const std::shared_ptr<PixelShader>& pixelShader)
	{
		m_pixelShader = pixelShader;

	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::GetVertexShader

	  Summary:  Returns the vertex shader

	  Returns:  ComPtr<ID3D11VertexShader>&
				  Vertex shader. Could be a nullptr
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	ComPtr<ID3D11VertexShader>& Renderable::GetVertexShader()
	{
		return m_vertexShader->GetVertexShader();

	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::GetPixelShader

	  Summary:  Returns the vertex shader

	  Returns:  ComPtr<ID3D11PixelShader>&
				  Pixel shader. Could be a nullptr
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	ComPtr<ID3D11PixelShader>& Renderable::GetPixelShader()
	{
		return m_pixelShader->GetPixelShader();

	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::GetVertexLayout

	  Summary:  Returns the vertex input layout

	  Returns:  ComPtr<ID3D11InputLayout>&
				  Vertex input layout
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	ComPtr<ID3D11InputLayout>& Renderable::GetVertexLayout()
	{
		return m_vertexShader->GetVertexLayout();

	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::GetVertexBuffer

	  Summary:  Returns the vertex buffer

	  Returns:  ComPtr<ID3D11Buffer>&
				  Vertex buffer
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	ComPtr<ID3D11Buffer>& Renderable::GetVertexBuffer()
	{
		return m_vertexBuffer;

	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::GetIndexBuffer

	  Summary:  Returns the index buffer

	  Returns:  ComPtr<ID3D11Buffer>&
				  Index buffer
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	ComPtr<ID3D11Buffer>& Renderable::GetIndexBuffer()
	{
		return m_indexBuffer;

	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::GetConstantBuffer

	  Summary:  Returns the constant buffer

	  Returns:  ComPtr<ID3D11Buffer>&
				  Constant buffer
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	ComPtr<ID3D11Buffer>& Renderable::GetConstantBuffer()
	{
		return m_constantBuffer;

	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
		  Method:   Renderable::GetWorldMatrix

		  Summary:  Returns the world matrix

		  Returns:  const XMMATRIX&
					  World matrix
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	const XMMATRIX& Renderable::GetWorldMatrix() const
	{
		return m_world;

	}



	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::GetOutputColor

	  Summary:  Returns the output color

	  Returns:  const XMFLOAT4&
				  The output color
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	const XMFLOAT4& Renderable::GetOutputColor() const
	{
		return m_outputColor;

	}
	/*M + M++ + M++ + M++ + M++ + M++ + M++ + M++ + M++ + M++ + M++ + M++ + M++ + M++ + M++ + M++ + M++ + M
	Method : Renderable::HasTexture

	Summary : Returns whether the renderable has texture

	Returns : BOOL
	Whether the renderable has texture
	M-- - M-- - M-- - M-- - M-- - M-- - M-- - M-- - M-- - M-- - M-- - M-- - M-- - M-- - M-- - M-- - M - M * /
	/*--------------------------------------------------------------------
		 TODO: Renderable::HasTexture definition (remove the comment)
	--------------------------------------------------------------------*/
	BOOL Renderable::HasTexture() const
	{
		if (m_aMaterials.size() > 0)
			return true;
		else
			return false;

	}


	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::GetMaterial

	  Summary:  Returns a material at given index

	  Returns:  const Material&
				  Material at given index
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	const Material& Renderable::GetMaterial(UINT uIndex) const
	{
		assert(uIndex < m_aMaterials.size());

		return m_aMaterials[uIndex];
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::GetMesh

	  Summary:  Returns a basic mesh entry at given index

	  Returns:  const Renderable::BasicMeshEntry&
				  Basic mesh entry at given index
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	const Renderable::BasicMeshEntry& Renderable::GetMesh(UINT uIndex) const
	{
		assert(uIndex < m_aMeshes.size());

		return m_aMeshes[uIndex];
	}


	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::RotateX
	  Summary:  Rotates around the x-axis
	  Args:     FLOAT angle
				  Angle of rotation around the x-axis, in radians
	  Modifies: [m_world].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderable::RotateX(_In_ FLOAT angle)
	{

		m_world *= XMMatrixRotationX(angle);

	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::RotateY
	  Summary:  Rotates around the y-axis
	  Args:     FLOAT angle
				  Angle of rotation around the y-axis, in radians
	  Modifies: [m_world].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderable::RotateY(_In_ FLOAT angle)
	{
		m_world *= XMMatrixRotationY(angle);

	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::RotateZ
	  Summary:  Rotates around the z-axis
	  Args:     FLOAT angle
				  Angle of rotation around the z-axis, in radians
	  Modifies: [m_world].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderable::RotateZ(_In_ FLOAT angle)
	{
		m_world *= XMMatrixRotationZ(angle);

	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::RotateRollPitchYaw
	  Summary:  Rotates based on a given pitch, yaw, and roll (Euler angles)
	  Args:     FLOAT pitch
				  Angle of rotation around the x-axis, in radians
				FLOAT yaw
				  Angle of rotation around the y-axis, in radians
				FLOAT roll
				  Angle of rotation around the z-axis, in radians
	  Modifies: [m_world].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderable::RotateRollPitchYaw(_In_ FLOAT pitch, _In_ FLOAT yaw, _In_ FLOAT roll)
	{
		m_world *= XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::Scale
	  Summary:  Scales along the x-axis, y-axis, and z-axis
	  Args:     FLOAT scaleX
				  Scaling factor along the x-axis.
				FLOAT scaleY
				  Scaling factor along the y-axis.
				FLOAT scaleZ
				  Scaling factor along the z-axis.
	  Modifies: [m_world].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderable::Scale(_In_ FLOAT scaleX, _In_ FLOAT scaleY, _In_ FLOAT scaleZ)
	{
		m_world *= XMMatrixScaling(scaleX, scaleY, scaleZ);

	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::Translate
	  Summary:  Translates matrix from a vector
	  Args:     const XMVECTOR& offset
				  3D vector describing the translations along the x-axis, y-axis, and z-axis
	  Modifies: [m_world].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	void Renderable::Translate(_In_ const XMVECTOR& offset)
	{
		m_world *= XMMatrixTranslationFromVector(offset);

	}
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::GetNumMeshes

	  Summary:  Returns the number of meshes

	  Returns:  UINT
				  Number of meshes
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	UINT Renderable::GetNumMeshes() const
	{
		return static_cast<UINT>(m_aMeshes.size());
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Renderable::GetNumMaterials

	  Summary:  Returns the number of materials

	  Returns:  UINT
				  Number of materials
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	UINT Renderable::GetNumMaterials() const
	{
		return static_cast<UINT>(m_aMaterials.size());
	}
}
