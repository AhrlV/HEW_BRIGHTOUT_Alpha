/*====================================================================

    リソースマネージメントのヘッダーまとめ [resource_management.h]

    Author : Ryosuke Kageyama
    Date   : 2025/12/03
====================================================================*/

#ifndef RESOURCE_MANAGEMENT_H
#define RESOURCE_MANAGEMENT_H
#include <memory>

#include "lifecycle/object.h"

#include "resourcemanagement/resource_manager.h"
#include "resourcemanagement/mesh.h"
#include "resourcemanagement/mesh_factory.h"
#include "resourcemanagement/material.h"
#include "resourcemanagement/material_factory.h"
#include "resourcemanagement/texture.h"
#include "resourcemanagement/texture_factory.h"
#include "resourcemanagement/shader.h"
#include "resourcemanagement/shader_factory.h"
#include "resourcemanagement/model_factory.h"
#include "resourcemanagement/prefab.h"
#include "resourcemanagement/constant_buffer.h"

using Meshptr     = std::shared_ptr<Mesh>;
using Textureptr  = std::shared_ptr<Texture>;
using Materialptr = std::shared_ptr<Material>;
using Shaderptr   = std::shared_ptr<Shader>;
using Modelptr    = std::shared_ptr<Model>;
using Prefabptr   = std::shared_ptr<Prefab>;

#endif
