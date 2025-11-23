/*==============================================================================

   Direct3D11用 デバックテキスト表示 [debug_text.cpp]
                                     Author : Youhei Sato
                                     Date   : 2025/06/15
--------------------------------------------------------------------------------

==============================================================================*/
#include "direct3D/debug_text.h"
#include "direct3D/WICTextureLoader11.h"
using namespace DirectX;
#include <D3Dcompiler.h>
#include <cstring>
#include <cassert>
using namespace Microsoft::WRL;

#pragma comment(lib, "d3dcompiler.lib")

namespace hal
{
	std::unordered_map<std::wstring, std::tuple<ID3D11Resource*, ID3D11ShaderResourceView*>> DebugText::m_TextureMap;
	ComPtr<ID3D11BlendState> DebugText::m_pBlendState;
	ComPtr<ID3D11DepthStencilState> DebugText::m_pDepthStencilState;
	ComPtr<ID3D11RasterizerState> DebugText::m_pRasterizerState;

	ComPtr<ID3D11VertexShader> DebugText::m_pVertexShader;
	ComPtr<ID3D11InputLayout> DebugText::m_pInputLayout;
	ComPtr<ID3D11Buffer> DebugText::m_pVSConstantBuffer;
	ComPtr<ID3D11PixelShader> DebugText::m_pPixelShader;
	ComPtr<ID3D11SamplerState> DebugText::m_pSamplerState;

	DebugText::DebugText(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const wchar_t* pFontTextureFileName, UINT screenWidth, UINT screenHeight, float offsetX, float offsetY, ULONG maxLine, ULONG maxCharactersPerLine, float lineSpacing, float characterSpacing)
		: m_pDevice(pDevice), m_pContext(pContext), m_FileName(pFontTextureFileName), m_OffsetX(offsetX), m_OffsetY(offsetY), m_MaxLine(maxLine), m_MaxCharactersPerLine(maxCharactersPerLine), m_LineSpacing(lineSpacing), m_CharacterSpacing(characterSpacing)
	{
		assert(m_pDevice && m_pContext);
		auto it = m_TextureMap.find(pFontTextureFileName);
		if (it != m_TextureMap.end()) {
			m_pTexture = std::get<0>(it->second);
			m_pTextureView = std::get<1>(it->second);
			m_pTexture->AddRef();
			m_pTextureView->AddRef();
		}
		else {
			HRESULT hr = CreateWICTextureFromFile(pDevice, pFontTextureFileName, &m_pTexture, &m_pTextureView);
			if (FAILED(hr) || !m_pTexture || !m_pTextureView) {
				MessageBoxW(nullptr, L"フォントテクスチャの読み込みに失敗しました", pFontTextureFileName, MB_OK | MB_ICONERROR);
				return; // 以降は描画不可
			}
			m_TextureMap[pFontTextureFileName] = std::make_tuple(m_pTexture, m_pTextureView);
		}

		if (m_pTexture) {
			D3D11_TEXTURE2D_DESC texture2d_desc{};
			static_cast<ID3D11Texture2D*>(m_pTexture)->GetDesc(&texture2d_desc);
			m_TextureWidth = texture2d_desc.Width;
			m_TextureHeight = texture2d_desc.Height;
		}

		if (!m_LineSpacing && m_TextureHeight) m_LineSpacing = m_TextureHeight / 16.0f;
		if (!m_CharacterSpacing && m_TextureWidth) m_CharacterSpacing = m_TextureWidth / 16.0f;

		m_TextLines.emplace_back();

		if (!m_pBlendState) {
			D3D11_BLEND_DESC blend_desc{};
			blend_desc.RenderTarget[0].BlendEnable = TRUE;
			blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			m_pDevice->CreateBlendState(&blend_desc, m_pBlendState.GetAddressOf());
		}

		if (!m_pDepthStencilState) {
			D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};
			depth_stencil_desc.DepthEnable = FALSE;
			depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;
			depth_stencil_desc.StencilEnable = FALSE;
			m_pDevice->CreateDepthStencilState(&depth_stencil_desc, m_pDepthStencilState.GetAddressOf());
		}

		if (!m_pRasterizerState) {
			D3D11_RASTERIZER_DESC rasterizer_desc{};
			rasterizer_desc.FillMode = D3D11_FILL_SOLID;
			rasterizer_desc.CullMode = D3D11_CULL_BACK;
			rasterizer_desc.DepthClipEnable = TRUE;
			m_pDevice->CreateRasterizerState(&rasterizer_desc, m_pRasterizerState.GetAddressOf());
		}

		if (!m_pVertexShader) {
			static const char* vs_text = R"(
				float4x4 mtx;
				struct VS_IN { float4 posL:POSITION0; float4 color:COLOR0; float2 uv:TEXCOORD0; };
				struct VS_OUT { float4 posH:SV_POSITION; float4 color:COLOR0; float2 uv:TEXCOORD0; };
				VS_OUT main(VS_IN vsin){ VS_OUT o; o.posH = mul(vsin.posL, mtx); o.color = vsin.color; o.uv = vsin.uv; return o; }
			)";
			ComPtr<ID3DBlob> pVSBlob, pErr;
			HRESULT hr = D3DCompile(vs_text, strlen(vs_text), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, pVSBlob.GetAddressOf(), pErr.GetAddressOf());
			if (FAILED(hr)) { if (pErr) OutputDebugStringA((char*)pErr->GetBufferPointer()); return; }
			m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf());
			D3D11_INPUT_ELEMENT_DESC layout[] = {
				{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
				{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
				{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,28,D3D11_INPUT_PER_VERTEX_DATA,0}
			};
			m_pDevice->CreateInputLayout(layout, (UINT)std::size(layout), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), m_pInputLayout.GetAddressOf());
		}

		if (!m_pVSConstantBuffer) {
			D3D11_BUFFER_DESC desc{}; desc.ByteWidth = sizeof(XMFLOAT4X4); desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; desc.Usage = D3D11_USAGE_DEFAULT;
			m_pDevice->CreateBuffer(&desc, nullptr, m_pVSConstantBuffer.GetAddressOf());
			XMFLOAT4X4 mtx; XMStoreFloat4x4(&mtx, XMMatrixTranspose(XMMatrixOrthographicOffCenterLH(0.f, (float)screenWidth, (float)screenHeight, 0.f, 0.f, 1.f)));
			if (m_pVSConstantBuffer) m_pContext->UpdateSubresource(m_pVSConstantBuffer.Get(), 0, nullptr, &mtx, 0, 0);
		}

		if (!m_pPixelShader) {
			static const char* ps_text = R"(
				struct PS_INPUT { float4 posH:SV_POSITION; float4 color:COLOR0; float2 uv:TEXCOORD0; };
				Texture2D fontTexture:register(t0); SamplerState fontSampler:register(s0);
				float4 main(PS_INPUT i):SV_TARGET { float4 c = fontTexture.Sample(fontSampler, i.uv); return c * i.color; }
			)";
			ComPtr<ID3DBlob> pPSBlob, pErr;
			HRESULT hr = D3DCompile(ps_text, strlen(ps_text), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, pPSBlob.GetAddressOf(), pErr.GetAddressOf());
			if (FAILED(hr)) { if (pErr) OutputDebugStringA((char*)pErr->GetBufferPointer()); return; }
			m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf());
		}

		if (!m_pSamplerState) {
			D3D11_SAMPLER_DESC samp{}; samp.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; samp.AddressU = samp.AddressV = samp.AddressW = D3D11_TEXTURE_ADDRESS_WRAP; samp.ComparisonFunc = D3D11_COMPARISON_ALWAYS; samp.MinLOD = 0.f; samp.MaxLOD = D3D11_FLOAT32_MAX;
			m_pDevice->CreateSamplerState(&samp, m_pSamplerState.GetAddressOf());
		}
	}

	DebugText::~DebugText()
	{
		if (m_pTextureView) {
			if (m_pTexture) m_pTexture->Release();
			if (!m_pTextureView->Release()) {
				m_TextureMap.erase(m_TextureMap.find(m_FileName));
			}
		}
	}

	void DebugText::SetText(const char* pText, XMFLOAT4 color)
	{
		if (m_TextLines.empty()) m_TextLines.emplace_back();
		m_TextLines.back().strings.emplace_back(color);
		for (; *pText; ++pText) {
			if (*pText == '\n') { m_TextLines.emplace_back(); m_TextLines.back().strings.emplace_back(color); }
			else if (*pText == '\r') { /* ignore */ }
			else if (*pText == '\t') {
				while (m_TextLines.back().characterCount % 4 != 0) {
					if (m_MaxCharactersPerLine && m_TextLines.back().characterCount >= m_MaxCharactersPerLine) { m_TextLines.emplace_back(); m_TextLines.back().strings.emplace_back(color); break; }
					m_TextLines.back().strings.back().characters += ' '; m_TextLines.back().characterCount++; m_TextLines.back().spaceCount++;
				}
			}
			else {
				if (m_MaxCharactersPerLine && m_TextLines.back().characterCount >= m_MaxCharactersPerLine) { m_TextLines.emplace_back(); m_TextLines.back().strings.emplace_back(color); }
				if (*pText >= ' ' && *pText <= '~') { m_TextLines.back().strings.back().characters += *pText; (*pText != ' ' ? m_CharacterCount++ : m_TextLines.back().spaceCount++); m_TextLines.back().characterCount++; }
				else { m_TextLines.back().strings.back().characters += '?'; m_CharacterCount++; m_TextLines.back().characterCount++; }
			}
		}
		if (m_MaxLine) {
			size_t effective = m_TextLines.size();
			if (m_TextLines.back().characterCount == 0 && effective > 0) effective--;
			while (effective > m_MaxLine && m_TextLines.size() > 1) {
				auto& front = m_TextLines.front(); ULONG removeCnt = front.characterCount - front.spaceCount; if (removeCnt <= m_CharacterCount) m_CharacterCount -= removeCnt; m_TextLines.pop_front(); effective = m_TextLines.size(); if (m_TextLines.back().characterCount == 0 && effective > 0) effective--; }
		}
	}

	void DebugText::Draw()
	{
		// 前提リソースが無ければ描画不可
		if (!m_CharacterCount) return;
		if (!m_pVertexShader || !m_pPixelShader || !m_pInputLayout || !m_pVSConstantBuffer || !m_pTextureView) return;
		if (!m_pVertexBuffer || m_CharacterCount > m_BufferSourceCharacterCount) createBuffer(m_CharacterCount);
		if (!m_pVertexBuffer || !m_pIndexBuffer) return;

		// 頂点/インデックス書き込み
		D3D11_MAPPED_SUBRESOURCE msrVB{};
		if (FAILED(m_pContext->Map(m_pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msrVB))) return;
		Vertex* v = static_cast<Vertex*>(msrVB.pData);
		D3D11_MAPPED_SUBRESOURCE msrIB{};
		if (FAILED(m_pContext->Map(m_pIndexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msrIB))) { m_pContext->Unmap(m_pVertexBuffer.Get(), 0); return; }
		WORD* indices = static_cast<WORD*>(msrIB.pData);

		UINT lineCount = 0; WORD glyphCount = 0; const float cw = m_TextureWidth / 16.f; const float ch = m_TextureHeight / 16.f;
		for (const auto& line : m_TextLines) {
			UINT col = 0;
			for (const auto& str : line.strings) {
				for (char code : str.characters) {
					int idx = code - ' ';
					if (idx) {
						float u0 = (idx % 16) / 16.f; float v0 = (idx / 16) / 16.f; float u1 = (idx % 16 + 1) / 16.f; float v1 = (idx / 16 + 1) / 16.f;
						float x = m_OffsetX + col * m_CharacterSpacing; float y = m_OffsetY + lineCount * m_LineSpacing;
						v[0].position = { x, y, 1.f }; v[0].color = str.color; v[0].texcoord = { u0, v0 };
						v[1].position = { x + cw, y, 1.f }; v[1].color = str.color; v[1].texcoord = { u1, v0 };
						v[2].position = { x, y + ch, 1.f }; v[2].color = str.color; v[2].texcoord = { u0, v1 };
						v[3].position = { x + cw, y + ch, 1.f }; v[3].color = str.color; v[3].texcoord = { u1, v1 };
						v += 4;
						indices[0] = glyphCount * 4 + 0; indices[1] = glyphCount * 4 + 1; indices[2] = glyphCount * 4 + 2; indices[3] = glyphCount * 4 + 2; indices[4] = glyphCount * 4 + 1; indices[5] = glyphCount * 4 + 3; indices += 6; glyphCount++;
					}
					col++;
				}
			}
			lineCount++;
		}

		m_pContext->Unmap(m_pVertexBuffer.Get(), 0);
		m_pContext->Unmap(m_pIndexBuffer.Get(), 0);

		UINT stride = sizeof(Vertex); UINT offset = 0;
		m_pContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
		m_pContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		m_pContext->IASetInputLayout(m_pInputLayout.Get());
		m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
		m_pContext->VSSetConstantBuffers(0, 1, m_pVSConstantBuffer.GetAddressOf());
		m_pContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
		m_pContext->PSSetShaderResources(0, 1, &m_pTextureView);
		m_pContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());

		ComPtr<ID3D11BlendState> prevBlend; float prevBlendFactor[4]; UINT prevMask; m_pContext->OMGetBlendState(prevBlend.GetAddressOf(), prevBlendFactor, &prevMask);
		float blendFactor[4] = {0,0,0,0}; m_pContext->OMSetBlendState(m_pBlendState.Get(), blendFactor, 0xffffffff);
		ComPtr<ID3D11DepthStencilState> prevDS; UINT prevRef = 0; m_pContext->OMGetDepthStencilState(prevDS.GetAddressOf(), &prevRef); m_pContext->OMSetDepthStencilState(m_pDepthStencilState.Get(), 0);
		ComPtr<ID3D11RasterizerState> prevRS; m_pContext->RSGetState(prevRS.GetAddressOf()); m_pContext->RSSetState(m_pRasterizerState.Get());

		m_pContext->DrawIndexed(m_CharacterCount * 6, 0, 0);

		// restore
		m_pContext->OMSetBlendState(prevBlend.Get(), prevBlendFactor, 0xffffffff);
		m_pContext->OMSetDepthStencilState(prevDS.Get(), prevRef);
		m_pContext->RSSetState(prevRS.Get());
	}

	void DebugText::Clear()
	{
		m_TextLines.clear(); m_TextLines.emplace_back(); m_CharacterCount = 0;
	}

	void DebugText::createBuffer(ULONG characterCount)
	{
		if (!m_pDevice) return;
		D3D11_BUFFER_DESC bd{}; bd.Usage = D3D11_USAGE_DYNAMIC; bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; bd.BindFlags = D3D11_BIND_VERTEX_BUFFER; bd.ByteWidth = sizeof(Vertex) * characterCount * 4; m_pDevice->CreateBuffer(&bd, nullptr, m_pVertexBuffer.ReleaseAndGetAddressOf());
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER; bd.ByteWidth = sizeof(WORD) * characterCount * 6; m_pDevice->CreateBuffer(&bd, nullptr, m_pIndexBuffer.ReleaseAndGetAddressOf());
		m_BufferSourceCharacterCount = characterCount;
	}
}