/*============================================================================================================

    リソースファクトリー [resource_factory.h]
    各種リソース（Mesh、Material、Texture、Model、Shader）の生成を管理するファクトリークラス群。
    ResourceManagerと連携して、リソースの一元管理とキャッシュを実現する。
    
    このヘッダーは互換性のために残されており、
    各ファクトリークラスの個別ヘッダーをインクルードします。

    Author : Ryosuke Kageyama
    Date   : 2025/11/26

=============================================================================================================*/
#ifndef RESOURCE_FACTORY_H
#define RESOURCE_FACTORY_H

// 各ファクトリークラスのヘッダーをインクルード
#include "direct3D/mesh_factory.h"
#include "direct3D/material_factory.h"
#include "direct3D/texture_factory.h"
#include "direct3D/model_factory.h"
#include "direct3D/shader_factory.h"

#endif // RESOURCE_FACTORY_H
