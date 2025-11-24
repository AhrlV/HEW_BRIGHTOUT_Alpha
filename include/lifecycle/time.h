#ifndef TIME_H
#define TIME_H
/*====================================================================

    Timeユーティリティ [time.h]

    Author : Ryosuke Kageyama
    Date   : 2025/11/19
====================================================================*/

#include <chrono>

class Time
{
public:
    using clock = std::chrono::steady_clock;

    // 経過時間（秒）
    static float DeltaTime() { return s_DeltaTime; }
    static float FixedDeltaTime() { return s_FixedDeltaTime; }
    static float TimeScale() { return s_TimeScale; }

    static void SetTimeScale(float scale) { s_TimeScale = scale; }
    static void SetFixedDeltaTime(float seconds) { s_FixedDeltaTime = seconds; }

    // フレーム開始時に呼ぶ
    static void BeginFrame()
    {
        auto now = clock::now();
        if (s_Initialized) {
            std::chrono::duration<float> dt = now - s_Last;
            s_DeltaTime = dt.count() * s_TimeScale;
        } else {
            s_Initialized = true;
            s_DeltaTime = 0.0f;
        }
        s_Last = now;
        s_Accumulator += s_DeltaTime;
    }

    // 固定ステップが溜まっているか
    static bool HasFixedStep()
    {
        return s_Accumulator >= s_FixedDeltaTime;
    }

    // 固定ステップを1消費
    static void ConsumeFixedStep()
    {
        if (s_Accumulator >= s_FixedDeltaTime)
            s_Accumulator -= s_FixedDeltaTime;
    }
private:
    static inline bool s_Initialized = false;
    static inline clock::time_point s_Last{};
    static inline float s_DeltaTime = 0.0f;
    static inline float s_FixedDeltaTime = 1.0f / 60.0f; // デフォルト60FPS
    static inline float s_TimeScale = 1.0f;
    static inline float s_Accumulator = 0.0f;
};

#endif
