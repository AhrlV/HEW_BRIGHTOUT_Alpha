/*=======================================================================================================


     ゴルフコースの制御[map.h]

                                                                              Author :  Ryosuke Kageyama
                                                                              Date   :  2025/11/04
========================================================================================================*/


#ifndef MAP_H
#define MAP_H

#include <DirectXMath.h>

class Block
{
public:
    enum TYPE
    {
        NONE,
        NORMAL,
        GRASS,
        DIRT,
        SAND,
        TYPE_MAX,
    };

private:
    DirectX::XMFLOAT3 m_position{ 0.0f, 0.0f, 0.0f };
    TYPE m_type{ TYPE::NORMAL };

public:
    Block(const DirectX::XMFLOAT3& position, const TYPE& type = TYPE::NORMAL)
        : m_position(position), m_type(type)
    {}

    void Draw() const;
};

void Map_Init();
void Map_Update();
void Map_Draw();
void Map_UnInit();



#endif
