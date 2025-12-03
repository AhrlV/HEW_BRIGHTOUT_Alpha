/*============================================================================================================

    メッシュファクトリー実装 [mesh_factory.cpp]
    Meshリソースの生成と管理を行うファクトリークラスの実装。

    Author : Ryosuke Kageyama
    Date   : 2025/11/26

=============================================================================================================*/
#include "resourcemanagement/mesh_factory.h"
#include "resourcemanagement/mesh.h"
#include "resourcemanagement/resource_manager.h"
#include "direct3D/direct3d_device.h"
#include <stdexcept>
#include <vector>
#include <sstream>
#include <iomanip>
#define _USE_MATH_DEFINES
#include <math.h>

/*========================================================================================================
	MeshFactory::Create
	
	メッシュの生成または取得を行う。
	ResourceManagerにキャッシュされていればそれを返し、
	なければファイルから読み込んでResourceManagerに登録する。
	キャッシュキーはファイル名のみで、フルパスは含まない。
	
	引数:
	  filename - メッシュファイルのパス
	戻り値: メッシュのshared_ptr
	例外: 読み込みに失敗した場合はruntime_errorをスロー
========================================================================================================*/
std::shared_ptr<Mesh> MeshFactory::Create(const std::wstring& filename)
{
	// GraphicsDeviceからDeviceを取得
	auto& device = GraphicsDevice::Instance();
	auto d3dDevice = device.GetDevice();
	
	if (!d3dDevice)
	{
		throw std::runtime_error("MeshFactory::Create - デバイスがnullです");
	}

	// ResourceManagerから取得を試行（ファイル名をキーとして使用）
	auto& manager = ResourceManager::Instance();
	auto resource = manager.GetResource<Mesh>(filename);
	
	if (resource)
	{
		// キャッシュされていたらそれを返す
		return resource;
	}
	
	// キャッシュされていない場合は新規作成
	auto mesh = std::make_shared<Mesh>(filename);
	
	// ファイルから読み込み（プレースホルダー）
	// TODO: 実際のメッシュ読み込み処理を実装する
	// ResourceManagerに登録（ファイル名をキーとして登録）
	manager.RegisterResource(filename, mesh);
	
	throw std::runtime_error("MeshFactory::Create - メッシュの読み込みは未実装です");
}

/*========================================================================================================
	MeshFactory::CreateCube
	
	立方体のプリミティブメッシュの生成または取得を行う。
	ResourceManagerに"P/Cube/s{size}"として登録・キャッシュされる。
	
	引数:
	  size - 立方体の一辺のサイズ
	戻り値: 立方体のメッシュのshared_ptr
	例外: 初期化に失敗した場合はruntime_errorをスロー
========================================================================================================*/
std::shared_ptr<Mesh> MeshFactory::CreateCube(float size)
{

	// キャッシュキーを生成（パラメータを含める）
	std::wstringstream ss;
	ss << L"P/Cube/s" << std::fixed << std::setprecision(3) << size;
	std::wstring cacheKey = ss.str();
	
	// ResourceManagerから取得を試行
	auto& manager = ResourceManager::Instance();
	auto resource = manager.GetResource<Mesh>(cacheKey);
	
	if (resource)
	{
		// キャッシュされていたらそれを返す
		return resource;
	}
	
	// 新規作成
	auto mesh = std::make_shared<Mesh>(cacheKey);
	
	// 立方体の頂点データ生成
	float halfSize = size * 0.5f;
	std::vector<MeshVertex> vertices;
	std::vector<uint32_t> indices;
	
	// 各面の頂点データ（24頂点：6面 × 4頂点）
	// 前面（+Z）
	vertices.push_back({ { -halfSize, -halfSize,  halfSize, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	vertices.push_back({ {  halfSize, -halfSize,  halfSize, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	vertices.push_back({ {  halfSize,  halfSize,  halfSize, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	vertices.push_back({ { -halfSize,  halfSize,  halfSize, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	
	// 背面（-Z）
	vertices.push_back({ {  halfSize, -halfSize, -halfSize, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	vertices.push_back({ { -halfSize, -halfSize, -halfSize, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	vertices.push_back({ { -halfSize,  halfSize, -halfSize, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	vertices.push_back({ {  halfSize,  halfSize, -halfSize, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	
	// 上面（+Y）
	vertices.push_back({ { -halfSize,  halfSize,  halfSize, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	vertices.push_back({ {  halfSize,  halfSize,  halfSize, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	vertices.push_back({ {  halfSize,  halfSize, -halfSize, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	vertices.push_back({ { -halfSize,  halfSize, -halfSize, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	
	// 下面（-Y）
	vertices.push_back({ { -halfSize, -halfSize, -halfSize, 1.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	vertices.push_back({ {  halfSize, -halfSize, -halfSize, 1.0f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	vertices.push_back({ {  halfSize, -halfSize,  halfSize, 1.0f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	vertices.push_back({ { -halfSize, -halfSize,  halfSize, 1.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	
	// 右面（+X）
	vertices.push_back({ {  halfSize, -halfSize,  halfSize, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	vertices.push_back({ {  halfSize, -halfSize, -halfSize, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	vertices.push_back({ {  halfSize,  halfSize, -halfSize, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	vertices.push_back({ {  halfSize,  halfSize,  halfSize, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	
	// 左面（-X）
	vertices.push_back({ { -halfSize, -halfSize, -halfSize, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	vertices.push_back({ { -halfSize, -halfSize,  halfSize, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	vertices.push_back({ { -halfSize,  halfSize,  halfSize, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	vertices.push_back({ { -halfSize,  halfSize, -halfSize, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	
	// インデックスデータ（6面 × 2三角形 × 3頂点）
	for (uint32_t i = 0; i < 6; ++i)
	{
		uint32_t baseIndex = i * 4;
		indices.push_back(baseIndex + 0);
		indices.push_back(baseIndex + 1);
		indices.push_back(baseIndex + 2);
		indices.push_back(baseIndex + 0);
		indices.push_back(baseIndex + 2);
		indices.push_back(baseIndex + 3);
	}
	
	// メッシュ初期化
	if (!mesh->Initialize(vertices, indices))
	{
		throw std::runtime_error("立方体のメッシュの初期化に失敗しました");
	}
	
	// ResourceManagerに登録
	manager.RegisterResource(cacheKey, mesh);
	
	return mesh;
}

/*========================================================================================================
	MeshFactory::CreateSphere
	
	球のプリミティブメッシュの生成または取得を行う。
	ResourceManagerに"P/Sphere/r{radius}/sl{slices}/st{stacks}"として登録・キャッシュされる。
	
	引数:
	  radius - 球の半径
	  slices - 経度方向の分割数
	  stacks - 緯度方向の分割数
	戻り値: 球のメッシュのshared_ptr
	例外: 初期化に失敗した場合はruntime_errorをスロー
========================================================================================================*/
std::shared_ptr<Mesh> MeshFactory::CreateSphere(float radius, int slices, int stacks)
{

	// キャッシュキーを生成（パラメータを含める）
	std::wstringstream ss;
	ss << L"P/Sphere/r" << std::fixed << std::setprecision(3) << radius 
	   << L"/sl" << slices 
	   << L"/st" << stacks;
	std::wstring cacheKey = ss.str();
	
	// ResourceManagerから取得を試行
	auto& manager = ResourceManager::Instance();
	auto resource = manager.GetResource<Mesh>(cacheKey);
	
	if (resource)
	{
		// キャッシュされていたらそれを返す
		return resource;
	}
	
	// 新規作成
	auto mesh = std::make_shared<Mesh>(cacheKey);
	
	// 球の頂点データ生成
	std::vector<MeshVertex> vertices;
	std::vector<uint32_t> indices;
	
	// 頂点生成
	for (int stack = 0; stack <= stacks; ++stack)
	{
		float phi = static_cast<float>(M_PI) * static_cast<float>(stack) / static_cast<float>(stacks);
		float y = radius * cosf(phi);
		float r = radius * sinf(phi);
		
		for (int slice = 0; slice <= slices; ++slice)
		{
			float theta = 2.0f * static_cast<float>(M_PI) * static_cast<float>(slice) / static_cast<float>(slices);
			float x = r * cosf(theta);
			float z = r * sinf(theta);
			
			DirectX::XMFLOAT4 position(x, y, z, 1.0f);
			DirectX::XMFLOAT3 normal(x / radius, y / radius, z / radius);
			DirectX::XMFLOAT2 texcoord(
				static_cast<float>(slice) / static_cast<float>(slices),
				static_cast<float>(stack) / static_cast<float>(stacks)
			);
			DirectX::XMFLOAT4 color(1.0f, 1.0f, 1.0f, 1.0f);
			
			vertices.push_back({ position, normal, texcoord, color });
		}
	}
	
	// インデックス生成
	for (int stack = 0; stack < stacks; ++stack)
	{
		for (int slice = 0; slice < slices; ++slice)
		{
			uint32_t current = stack * (slices + 1) + slice;
			uint32_t next = current + slices + 1;
			
			indices.push_back(current);
			indices.push_back(next);
			indices.push_back(current + 1);
			
			indices.push_back(current + 1);
			indices.push_back(next);
			indices.push_back(next + 1);
		}
	}
	
	// メッシュ初期化
	if (!mesh->Initialize(vertices, indices))
	{
		throw std::runtime_error("球のメッシュの初期化に失敗しました");
	}
	
	// ResourceManagerに登録
	manager.RegisterResource(cacheKey, mesh);
	
	return mesh;
}

/*========================================================================================================
	MeshFactory::CreateCylinder
	
	円柱プリミティブメッシュの生成または取得を行う。
	ResourceManagerに"P/Cylinder/r{radius}/h{height}/sl{slices}"として登録・キャッシュされる。
	
	引数:
	  radius - 円柱の半径
	  height - 円柱の高さ
	  slices - 円周方向の分割数
	戻り値: 円柱メッシュのshared_ptr
	例外: 初期化に失敗した場合はruntime_errorをスロー
========================================================================================================*/
std::shared_ptr<Mesh> MeshFactory::CreateCylinder(float radius, float height, int slices)
{

	// キャッシュキーを生成（パラメータを含める）
	std::wstringstream ss;
	ss << L"P/Cylinder/r" << std::fixed << std::setprecision(3) << radius 
	   << L"/h" << std::fixed << std::setprecision(3) << height 
	   << L"/sl" << slices;
	std::wstring cacheKey = ss.str();
	
	// ResourceManagerから取得を試行
	auto& manager = ResourceManager::Instance();
	auto resource = manager.GetResource<Mesh>(cacheKey);
	
	if (resource)
	{
		// キャッシュされていたらそれを返す
		return resource;
	}
	
	// 新規作成
	auto mesh = std::make_shared<Mesh>(cacheKey);
	
	// 円柱の頂点データ生成
	std::vector<MeshVertex> vertices;
	std::vector<uint32_t> indices;
	
	float halfHeight = height * 0.5f;
	
	// 側面の頂点生成
	for (int i = 0; i <= slices; ++i)
	{
		float theta = 2.0f * static_cast<float>(M_PI) * static_cast<float>(i) / static_cast<float>(slices);
		float x = radius * cosf(theta);
		float z = radius * sinf(theta);
		
		DirectX::XMFLOAT3 normal(x / radius, 0.0f, z / radius);
		DirectX::XMFLOAT2 texcoord(static_cast<float>(i) / static_cast<float>(slices), 0.0f);
		DirectX::XMFLOAT4 color(1.0f, 1.0f, 1.0f, 1.0f);
		
		// 上部の頂点
		vertices.push_back({ { x, halfHeight, z, 1.0f }, normal, texcoord, color });
		
		// 下部の頂点
		texcoord.y = 1.0f;
		vertices.push_back({ { x, -halfHeight, z, 1.0f }, normal, texcoord, color });
	}
	
	// 側面のインデックス生成
	for (int i = 0; i < slices; ++i)
	{
		uint32_t current = i * 2;
		uint32_t next = current + 2;
		
		indices.push_back(current);
		indices.push_back(next);
		indices.push_back(current + 1);
		
		indices.push_back(current + 1);
		indices.push_back(next);
		indices.push_back(next + 1);
	}
	
	// 上面の頂点とインデックス
	uint32_t topCenterIndex = static_cast<uint32_t>(vertices.size());
	vertices.push_back({ { 0.0f, halfHeight, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	
	for (int i = 0; i < slices; ++i)
	{
		float theta = 2.0f * static_cast<float>(M_PI) * static_cast<float>(i) / static_cast<float>(slices);
		float x = radius * cosf(theta);
		float z = radius * sinf(theta);
		
		DirectX::XMFLOAT2 texcoord(
			0.5f + 0.5f * cosf(theta),
			0.5f + 0.5f * sinf(theta)
		);
		
		vertices.push_back({ { x, halfHeight, z, 1.0f }, { 0.0f, 1.0f, 0.0f }, texcoord, { 1.0f, 1.0f, 1.0f, 1.0f } });
	}
	
	for (int i = 0; i < slices; ++i)
	{
		uint32_t current = topCenterIndex + 1 + i;
		uint32_t next = topCenterIndex + 1 + ((i + 1) % slices);
		
		indices.push_back(topCenterIndex);
		indices.push_back(current);
		indices.push_back(next);
	}
	
	// 下面の頂点とインデックス
	uint32_t bottomCenterIndex = static_cast<uint32_t>(vertices.size());
	vertices.push_back({ { 0.0f, -halfHeight, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { 0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
	
	for (int i = 0; i < slices; ++i)
	{
		float theta = 2.0f * static_cast<float>(M_PI) * static_cast<float>(i) / static_cast<float>(slices);
		float x = radius * cosf(theta);
		float z = radius * sinf(theta);
		
		DirectX::XMFLOAT2 texcoord(
			0.5f + 0.5f * cosf(theta),
			0.5f - 0.5f * sinf(theta)
		);
		
		vertices.push_back({ { x, -halfHeight, z, 1.0f }, { 0.0f, -1.0f, 0.0f }, texcoord, { 1.0f, 1.0f, 1.0f, 1.0f } });
	}
	
	for (int i = 0; i < slices; ++i)
	{
		uint32_t current = bottomCenterIndex + 1 + i;
		uint32_t next = bottomCenterIndex + 1 + ((i + 1) % slices);
		
		indices.push_back(bottomCenterIndex);
		indices.push_back(next);
		indices.push_back(current);
	}
	
	// メッシュ初期化
	if (!mesh->Initialize(vertices, indices))
	{
		throw std::runtime_error("円柱メッシュの初期化に失敗しました");
	}
	
	// ResourceManagerに登録
	manager.RegisterResource(cacheKey, mesh);
	
	return mesh;
}

/*========================================================================================================
	MeshFactory::CreateCapsule
	
	カプセルプリミティブメッシュの生成または取得を行う。
	ResourceManagerに"P/Capsule/r{radius}/h{height}/sl{slices}/st{stacks}"として登録・キャッシュされる。
	
	引数:
	  radius - カプセルの半径
	  height - カプセルの円柱部分の高さ
	  slices - 円周方向の分割数
	  stacks - 球の半球部分の分割数
	戻り値: カプセルメッシュのshared_ptr
	例外: 初期化に失敗した場合はruntime_errorをスロー
========================================================================================================*/
std::shared_ptr<Mesh> MeshFactory::CreateCapsule(float radius, float height, int slices, int stacks)
{

	// キャッシュキーを生成（パラメータを含める）
	std::wstringstream ss;
	ss << L"P/Capsule/r" << std::fixed << std::setprecision(3) << radius 
	   << L"/h" << std::fixed << std::setprecision(3) << height 
	   << L"/sl" << slices 
	   << L"/st" << stacks;
	std::wstring cacheKey = ss.str();
	
	// ResourceManagerから取得を試行
	auto& manager = ResourceManager::Instance();
	auto resource = manager.GetResource<Mesh>(cacheKey);
	
	if (resource)
	{
		// キャッシュされていたらそれを返す
		return resource;
	}
	
	// 新規作成
	auto mesh = std::make_shared<Mesh>(cacheKey);
	
	// カプセルの頂点データ生成
	std::vector<MeshVertex> vertices;
	std::vector<uint32_t> indices;
	
	float halfHeight = height * 0.5f;
	
	// 上半球の頂点生成
	for (int stack = 0; stack <= stacks; ++stack)
	{
		float phi = static_cast<float>(M_PI) * 0.5f * static_cast<float>(stack) / static_cast<float>(stacks);
		float y = radius * cosf(phi) + halfHeight;
		float r = radius * sinf(phi);
		
		for (int slice = 0; slice <= slices; ++slice)
		{
			float theta = 2.0f * static_cast<float>(M_PI) * static_cast<float>(slice) / static_cast<float>(slices);
			float x = r * cosf(theta);
			float z = r * sinf(theta);
			
			DirectX::XMFLOAT4 position(x, y, z, 1.0f);
			DirectX::XMFLOAT3 normal(x / radius, (y - halfHeight) / radius, z / radius);
			DirectX::XMFLOAT2 texcoord(
				static_cast<float>(slice) / static_cast<float>(slices),
				static_cast<float>(stack) / static_cast<float>(stacks) * 0.25f
			);
			DirectX::XMFLOAT4 color(1.0f, 1.0f, 1.0f, 1.0f);
			
			vertices.push_back({ position, normal, texcoord, color });
		}
	}
	
	// 円柱部分の頂点生成
	for (int i = 0; i <= slices; ++i)
	{
		float theta = 2.0f * static_cast<float>(M_PI) * static_cast<float>(i) / static_cast<float>(slices);
		float x = radius * cosf(theta);
		float z = radius * sinf(theta);
		
		DirectX::XMFLOAT3 normal(x / radius, 0.0f, z / radius);
		DirectX::XMFLOAT2 texcoord(static_cast<float>(i) / static_cast<float>(slices), 0.25f);
		DirectX::XMFLOAT4 color(1.0f, 1.0f, 1.0f, 1.0f);
		
		vertices.push_back({ { x, halfHeight, z, 1.0f }, normal, texcoord, color });
		
		texcoord.y = 0.75f;
		vertices.push_back({ { x, -halfHeight, z, 1.0f }, normal, texcoord, color });
	}
	
	// 下半球の頂点生成
	uint32_t lowerHemisphereStart = static_cast<uint32_t>(vertices.size());
	for (int stack = 0; stack <= stacks; ++stack)
	{
		float phi = static_cast<float>(M_PI) * 0.5f * static_cast<float>(stack) / static_cast<float>(stacks);
		float y = -radius * cosf(phi) - halfHeight;
		float r = radius * sinf(phi);
		
		for (int slice = 0; slice <= slices; ++slice)
		{
			float theta = 2.0f * static_cast<float>(M_PI) * static_cast<float>(slice) / static_cast<float>(slices);
			float x = r * cosf(theta);
			float z = r * sinf(theta);
			
			DirectX::XMFLOAT4 position(x, y, z, 1.0f);
			DirectX::XMFLOAT3 normal(x / radius, (y + halfHeight) / radius, z / radius);
			DirectX::XMFLOAT2 texcoord(
				static_cast<float>(slice) / static_cast<float>(slices),
				0.75f + static_cast<float>(stack) / static_cast<float>(stacks) * 0.25f
			);
			DirectX::XMFLOAT4 color(1.0f, 1.0f, 1.0f, 1.0f);
			
			vertices.push_back({ position, normal, texcoord, color });
		}
	}
	
	// 上半球のインデックス生成
	for (int stack = 0; stack < stacks; ++stack)
	{
		for (int slice = 0; slice < slices; ++slice)
		{
			uint32_t current = stack * (slices + 1) + slice;
			uint32_t next = current + slices + 1;
			
			indices.push_back(current);
			indices.push_back(next);
			indices.push_back(current + 1);
			
			indices.push_back(current + 1);
			indices.push_back(next);
			indices.push_back(next + 1);
		}
	}
	
	// 円柱部分のインデックス生成
	uint32_t cylinderStart = (stacks + 1) * (slices + 1);
	for (int i = 0; i < slices; ++i)
	{
		uint32_t current = cylinderStart + i * 2;
		uint32_t next = current + 2;
		
		indices.push_back(current);
		indices.push_back(next);
		indices.push_back(current + 1);
		
		indices.push_back(current + 1);
		indices.push_back(next);
		indices.push_back(next + 1);
	}
	
	// 下半球のインデックス生成
	for (int stack = 0; stack < stacks; ++stack)
	{
		for (int slice = 0; slice < slices; ++slice)
		{
			uint32_t current = lowerHemisphereStart + stack * (slices + 1) + slice;
			uint32_t next = current + slices + 1;
			
			indices.push_back(current);
			indices.push_back(next);
			indices.push_back(current + 1);
			
			indices.push_back(current + 1);
			indices.push_back(next);
			indices.push_back(next + 1);
		}
	}
	
	// メッシュ初期化
	if (!mesh->Initialize(vertices, indices))
	{
		throw std::runtime_error("カプセルメッシュの初期化に失敗しました");
	}
	
	// ResourceManagerに登録
	manager.RegisterResource(cacheKey, mesh);
	
	return mesh;
}
