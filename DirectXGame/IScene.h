#pragma once
#include <optional>

enum class SceneID { kTitle, kGame, kResult };

// シーンの基底クラス
class IScene {
public:
	enum ScenePhase { kFadeIn, kMain, kFadeOut };

	virtual ~IScene() {}                         // デストラクタ
	virtual void Initialize() = 0;               // 初期化
	virtual std::optional<SceneID> Update() = 0; // 更新
	virtual void Draw() = 0;                     // 描画

protected:
	ScenePhase phase_ = ScenePhase::kFadeIn;

	int fadeTimer_ = 30;
	const int kFadeDuration_ = 30;
};