
/* ＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝

     スプライトアニメーションの描画

＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝*/

#ifndef SPRITEANIM_ANIM_H
#define SPRITEANIM_ANIM_H

#include <DirectXMath.h>



class AnimPattern
{
private:
    int m_texture_id;
    int m_pattern_max;
    int m_horizontal_pattern_max;

    double m_second_per_pattern;
    DirectX::XMUINT2 m_start_position;
    DirectX::XMUINT2 m_pattern_size; // x->w y->h
    bool m_is_loop;


public:
    AnimPattern() = delete;
    AnimPattern(int texid, int pattern_max, double second_per_pattern, int holizontal_pattern_max, DirectX::XMUINT2 start_postion, DirectX::XMUINT2 pattern_size, bool is_loop)
        : m_texture_id(texid), m_pattern_max(pattern_max), m_horizontal_pattern_max(holizontal_pattern_max), m_second_per_pattern(second_per_pattern), m_start_position(start_postion), m_pattern_size(pattern_size), m_is_loop(is_loop)
    {
    }

    int GetPatternMax() { return m_pattern_max; }
    double GetSecondPerPattern() { return m_second_per_pattern; }
    bool IsLoop() { return m_is_loop; }

    void Draw(float dx, float dy, float dw, float dh, int pattern_num, DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f });
};


class AnimPatternPlayer
{
private:
    int m_pattern = 0;
    double m_accumulated_time = 0;
    AnimPattern* m_pAnimPattern = nullptr;

public:
    AnimPatternPlayer() = delete;
    AnimPatternPlayer(AnimPattern* pAnimPattern)
        : m_pAnimPattern(pAnimPattern) {
    }

    void Update(double elapsed_time);
    void Draw(float dx, float dy, float dw, float dh, DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f });
};

#endif
