/*============================================================================================================

    リソースマネージャー実装 [resource_manager.cpp]
    リソースの一元管理を行うシングルトンクラスの実装。

    Author : Ryosuke Kageyama
    Date   : 2025/05/07

=============================================================================================================*/

#include "resourcemanagement/resource_manager.h"
#include <stdexcept>
#include <Windows.h>

/*============================================================================================================
    シングルトンインスタンス取得
    スレッドセーフな静的ローカル変数によるシングルトン実装。
    戻り値: ResourceManagerの唯一のインスタンスへの参照
=============================================================================================================*/
ResourceManager& ResourceManager::Instance()
{
    static ResourceManager instance;
    return instance;
}

/*============================================================================================================
    デストラクタ
    すべてのリソースを解放する。
=============================================================================================================*/
ResourceManager::~ResourceManager()
{
    Clear();
}

/*============================================================================================================
    リソース登録
    新しいリソースをマネージャーに登録する。
    同一ファイル名のリソースが既に登録されている場合は登録せず、falseを返す。
    GameObjectとComponentは静的リソースではないため登録できない。
    
    引数:
      filename - リソースのファイル名（キーとして使用）
      resource - 登録するリソースのshared_ptr
    戻り値: 登録に成功した場合true、既に登録済みまたはエラーの場合false
    例外: GameObjectまたはComponentを登録しようとした場合は例外をスロー
=============================================================================================================*/
bool ResourceManager::RegisterResource(const std::wstring& filename, std::shared_ptr<Object> resource)
{
    // ファイル名が空の場合はエラー
    if (filename.empty())
    {
        throw std::runtime_error("ResourceManager::RegisterResource - ファイル名が空です");
    }

    // リソースがnullptrの場合はエラー
    if (!resource)
    {
        throw std::runtime_error("ResourceManager::RegisterResource - リソースがnullptrです");
    }

    // GameObjectまたはComponentは登録不可（StaticResourceBorderより後のクラスID）
    ResourceClassID classID = resource->GetClassID();
    if (classID >= ResourceClassID::StaticResourceBorder)
    {
        throw std::runtime_error("ResourceManager::RegisterResource - GameObjectとComponentは登録できません");
    }

    // 既に同じファイル名のリソースが登録されているかチェック
    if (m_Resources.find(filename) != m_Resources.end())
    {
        // wstringをstringに変換してエラーメッセージを作成
        std::string filenameStr;
        filenameStr.reserve(filename.size());
        for (wchar_t wc : filename)
        {
            filenameStr.push_back(static_cast<char>(wc & 0xFF));
        }
        throw std::runtime_error("ResourceManager::RegisterResource - 同一ファイル名のリソースが既に登録されています: " + filenameStr);
    }

    // リソースを登録
    m_Resources[filename] = resource;
    
    return true;
}

/*============================================================================================================
    リソース取得
    登録されているリソースをファイル名から取得する。
    
    引数:
      filename - 取得するリソースのファイル名
    戻り値: 取得したリソースのshared_ptr（リソースが存在しない場合はnullptr）
=============================================================================================================*/
std::shared_ptr<Object> ResourceManager::GetResource(const std::wstring& filename) const
{
	auto it = m_Resources.find(filename);
	if (it != m_Resources.end())
	{
		return it->second;
	}
	return nullptr;
}

/*============================================================================================================
    リソース存在確認
    指定したファイル名のリソースが登録されているかを確認する。
    
    引数:
      filename - 確認するリソースのファイル名
    戻り値: リソースが存在する場合true、存在しない場合false
=============================================================================================================*/
bool ResourceManager::HasResource(const std::wstring& filename) const
{
    return m_Resources.find(filename) != m_Resources.end();
}

/*============================================================================================================
    リソース削除
    指定したファイル名のリソースをマネージャーから削除する。
    
    引数:
      filename - 削除するリソースのファイル名
    戻り値: 削除に成功した場合true、リソースが存在しなかった場合false
=============================================================================================================*/
bool ResourceManager::RemoveResource(const std::wstring& filename)
{
    auto it = m_Resources.find(filename);
    if (it == m_Resources.end())
    {
        return false;
    }
    
    // マップから削除
    m_Resources.erase(it);
    
    return true;
}

/*============================================================================================================
    クラスID指定リソース削除
    指定したクラスIDのリソースをすべて削除する。
    
    引数:
      classID - 削除するリソースのクラスID
    戻り値: 削除したリソース数
=============================================================================================================*/
size_t ResourceManager::RemoveResourcesByClassID(ResourceClassID classID)
{
    size_t removedCount = 0;
    
    // イテレータを使用して安全に削除
    for (auto it = m_Resources.begin(); it != m_Resources.end();)
    {
        if (it->second && it->second->GetClassID() == classID)
        {
            // マップから削除
            it = m_Resources.erase(it);
            
            removedCount++;
        }
        else
        {
            ++it;
        }
    }
    
    return removedCount;
}

/*============================================================================================================
    全リソース削除
    登録されているすべてのリソースをマネージャーから削除する。
=============================================================================================================*/
void ResourceManager::Clear()
{
    // マップをクリア
    m_Resources.clear();
}

/*============================================================================================================
    総リソース数取得
    登録されているリソースの総数を取得する。
    
    戻り値: 登録されているリソースの総数
=============================================================================================================*/
size_t ResourceManager::GetResourceCount() const
{
    return m_Resources.size();
}

/*============================================================================================================
    クラスID指定リソース数取得
    指定したクラスIDのリソース数を取得する。
    
    引数:
      classID - カウントするリソースのクラスID
    戻り値: 指定したクラスIDのリソース数
=============================================================================================================*/
size_t ResourceManager::GetResourceCountByClassID(ResourceClassID classID) const
{
    size_t count = 0;
    
    for (const auto& pair : m_Resources)
    {
        if (pair.second && pair.second->GetClassID() == classID)
        {
            count++;
        }
    }
    
    return count;
}

/*============================================================================================================
    デバッグ出力
    登録されているすべてのリソースの情報をデバッグ出力する。
    （実際のプロジェクトではロギングシステムを使用することを推奨）
=============================================================================================================*/
void ResourceManager::DumpResourceInfo() const
{
    // OutputDebugStringを使用してデバッグ出力
    // TODO: 実際のプロジェクトではロギングシステムを使用
    
    std::wstring output = L"=== Resource Manager Debug Info ===\n";
    output += L"Total Resources: " + std::to_wstring(m_Resources.size()) + L"\n\n";
    
    // クラスID別にカウント
    size_t counts[static_cast<size_t>(ResourceClassID::Count)] = {};
    
    for (const auto& pair : m_Resources)
    {
        if (pair.second)
        {
            size_t index = static_cast<size_t>(pair.second->GetClassID());
            if (index < static_cast<size_t>(ResourceClassID::Count))
            {
                counts[index]++;
            }
            
            // 各リソースの情報を出力
            output += L"  " + pair.first + L" (";
            
            switch (pair.second->GetClassID())
            {
            case ResourceClassID::Mesh:
                output += L"Mesh";
                break;
            case ResourceClassID::Material:
                output += L"Material";
                break;
            case ResourceClassID::Texture:
                output += L"Texture";
                break;
            case ResourceClassID::Model:
                output += L"Model";
                break;
            case ResourceClassID::Shader:
                output += L"Shader";
                break;
            case ResourceClassID::Audio:
                output += L"Audio";
                break;
            case ResourceClassID::GameObject:
                output += L"GameObject";
                break;
            case ResourceClassID::Component:
                output += L"Component";
                break;
            default:
                output += L"Unknown";
                break;
            }
            
            output += L")\n";
        }
    }
    
    output += L"\n=== Static Resources (Managed by ResourceManager) ===\n";
    output += L"  Mesh: " + std::to_wstring(counts[static_cast<size_t>(ResourceClassID::Mesh)]) + L"\n";
    output += L"  Material: " + std::to_wstring(counts[static_cast<size_t>(ResourceClassID::Material)]) + L"\n";
    output += L"  Texture: " + std::to_wstring(counts[static_cast<size_t>(ResourceClassID::Texture)]) + L"\n";
    output += L"  Model: " + std::to_wstring(counts[static_cast<size_t>(ResourceClassID::Model)]) + L"\n";
    output += L"  Shader: " + std::to_wstring(counts[static_cast<size_t>(ResourceClassID::Shader)]) + L"\n";
    output += L"  Audio: " + std::to_wstring(counts[static_cast<size_t>(ResourceClassID::Audio)]) + L"\n";
    output += L"\n=== Dynamic Objects (NOT Managed by ResourceManager) ===\n";
    output += L"  GameObject: " + std::to_wstring(counts[static_cast<size_t>(ResourceClassID::GameObject)]) + L" (should be 0)\n";
    output += L"  Component: " + std::to_wstring(counts[static_cast<size_t>(ResourceClassID::Component)]) + L" (should be 0)\n";
    output += L"=====================================\n";
    
    OutputDebugStringW(output.c_str());
}
