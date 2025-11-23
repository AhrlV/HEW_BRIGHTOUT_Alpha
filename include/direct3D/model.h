
#ifndef MODEL_H
#define MODEL_H


#include <unordered_map>
#include <d3d11.h>
#include <DirectXMath.h>

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/matrix4x4.h"
#pragma comment (lib, "assimp/assimp-vc143-mt.lib")



struct MODEL
{
	const aiScene* AiScene = nullptr;

	ID3D11Buffer** VertexBuffer;
	ID3D11Buffer** IndexBuffer;

	std::unordered_map<std::string, ID3D11ShaderResourceView*> Texture;
};


MODEL* ModelLoad(const char* FileName, float scale);
void ModelRelease(MODEL* model);

void ModelDraw(const MODEL* model, const DirectX::XMMATRIX& mtxWorld);

#endif
