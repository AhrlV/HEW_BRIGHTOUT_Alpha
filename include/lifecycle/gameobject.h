#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
/*====================================================================


	GameObjectクラス [gameobject.h]

								Author : Ryosuke Kageyama
								Date   : 2025/11/18
====================================================================*/

#include <vector>
#include <memory>
#include <type_traits>
#include <unordered_set>
#include "component.h"

class GameObject 
{

private:
	// 所有するComponentのvector（更新はこれを走査）
    std::vector<std::unique_ptr<Component>> m_Components;

	// 新規追加されたComponentを追跡するためのvector
    std::vector<Component*> m_AddedComponents;

	// Startが呼ばれたComponentを追跡するためのset
    std::unordered_set<Component*> m_StartedComponents;

    // このGameObject自身の有効/無効
    bool m_ActiveSelf = true;

public:
    GameObject();
    virtual ~GameObject();

    // static Create() は廃止。コンストラクタ内で登録する。

    // 有効/無効の設定・取得
    void SetActive(bool active);
    bool IsActiveSelf() const;
    // 階層が無いので現状は自己フラグに等しい。親子を導入したら伝播を考慮。
    bool IsActiveInHierarchy() const;

	// まだAwakeが呼ばれていない新規追加Componentに対してAwakeを呼び出す
    void AwakeNewComponents();

	// まだStartが呼ばれていないComponentに対してStartを呼び出す
    void StartNewComponents();

	// 全てのComponentに対してライフサイクルメソッドを呼び出す
    void Update();
    void LateUpdate();
    void FixedUpdate();

	// 全てのComponentに対してRenderを呼び出す
    void Render();



    // ComponentをGameObjectに追加するテンプレートメソッド
    template <typename T, typename... Args>
    T* AddComponent(Args&&... args)
    {
        // コンパイル時にTがComponentを継承していることを確認
        static_assert(std::is_base_of<Component, T>::value, "Component継承している型を指定してください");

        // Componentを生成
        auto comp = std::make_unique<T>(std::forward<Args>(args)...);

        // unique_ptrから生ポインタを取得
        T* raw = comp.get();

        // Componentの所有者を設定
        raw->m_Owner = this;

        // emplace_backを使いコピーを避けてComponentを追加
        m_Components.emplace_back(std::move(comp));

        // 新規追加されたComponentを追跡
        m_AddedComponents.push_back(raw);

        return raw;
    }

    // 指定した型のComponentを取得するテンプレートメソッド
    template <typename T>
    [[nodiscard]] T* GetComponent() const
    {
        for (auto& c : m_Components) {
            // dynamic_castを使って型をチェック
            if (auto casted = dynamic_cast<T*>(c.get())) return casted;
        }
        return nullptr;
    }

    // 指定した型の全てのComponentを取得するテンプレートメソッド
    template <typename T>
    [[nodiscard]] std::vector<T*> GetComponents() const
    {
        std::vector<T*> results;
        for (auto& c : m_Components) {
            if (auto casted = dynamic_cast<T*>(c.get())) results.push_back(casted);
        }
        // 空でも空配列を返す
        return results;
    }
};

#endif
