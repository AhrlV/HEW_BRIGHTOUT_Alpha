#include "sprite_anim.h"
#include "sprite.h"



void AnimPattern::Draw(float dx, float dy, float dw, float dh, int pattern_num, DirectX::XMFLOAT4 color)
{


	Sprite_Draw(m_texture_id, dx, dy, dw, dh, 
		m_start_position.x + m_pattern_size.x * (pattern_num % m_horizontal_pattern_max),
		m_start_position.y + m_pattern_size.y * (pattern_num / m_horizontal_pattern_max),
		m_pattern_size.x, m_pattern_size.y, color);
}


void AnimPatternPlayer::Update(double elapsed_time)
{
	m_accumulated_time += elapsed_time;

	if (m_accumulated_time >= m_pAnimPattern->GetSecondPerPattern()) {
		m_pattern = (m_pattern + 1) % m_pAnimPattern->GetPatternMax();
		m_accumulated_time -= m_pAnimPattern->GetSecondPerPattern();
	}
}


void AnimPatternPlayer::Draw(float dx, float dy, float dw, float dh, DirectX::XMFLOAT4 color)
{
	m_pAnimPattern->Draw(dx, dy, dw, dh, m_pattern, color);
}
