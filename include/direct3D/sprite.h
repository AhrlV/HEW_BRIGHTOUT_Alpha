/*==============================================================================

   スプライト [sprite.h]
														 Author : Youhei Sato
														 Date   : 2025/05/15
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef SPRITE_H
#define SPRITE_H

#include <d3d11.h>
#include <DirectXMath.h>


bool Sprite_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Sprite_Finalize(void);
void Sprite_Draw(int tex_id, float x, float y, float w, float h, DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f });
void Sprite_Draw(int tex_id, float x, float y, float w, float h, int tx, int ty, int tw, int th, DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f });
void Sprite_Draw(int tex_id, float x, float y, float w, float h, float angle, int tx, int ty, int tw, int th, DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f });


#endif // SPRITE_H
