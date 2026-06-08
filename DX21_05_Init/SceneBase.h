#pragma once

// ==========================================
//           すべてのシーンの基底クラス
// ==========================================

class SceneBase 
{
public:
    // 純粋仮想クラスなのでコンストラクタは不要。
    //virtual ~SceneBase() {} // 専用の破棄処理がないため空実装でもよい。
                            // 現在はこのクラス用の .cpp は存在しない。

    virtual bool Init() = 0;    
    virtual void Update(float deltaTime) = 0;
    virtual void Draw() = 0;
    virtual void Uninit() = 0;
};