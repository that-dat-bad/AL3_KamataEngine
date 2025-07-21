#pragma once
#include "Fade.h"
#include "KamataEngine.h"

using namespace KamataEngine;

class TitleScene {
public:
	// デストラクタ
	~TitleScene();

	// 初期化
	void Initialize();
	// 更新
	void Update();
	// 描画
	void Draw();

	// 終了したかを取得
	bool IsFinished() const { return finished_; }

private:
	enum class Phase {
		kFadeIn,  // フェードイン
		kMain,    // メイン処理
		kFadeOut, // フェードアウト

	};
	// 現在のフェーズ
	Phase phase_;
	// フェード
	Fade* fade_ = nullptr;

	static inline const float kFadeDuration = 1.0f;

	// 終了フラグ
	bool finished_ = false;
};