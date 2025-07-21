#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

class Fade {
public:
	// フェードの状態
	enum class Status {
		None,    // フェードなし
		FadeIn,  // フェードイン中
		FadeOut, // フェードアウト中
	};

public:
	// 初期化
	void Initialize();
	// 更新
	void Update();
	// 描画
	void Draw();
	// フェード開始
	void Start(Status status, float duration);
	// フェード停止
	void Stop();
	// フェードが終了したか
	bool IsFinished() const;


private:
	// フェード用の黒いスプライト
	Sprite* sprite_ = nullptr;

	// 現在のフェードの状態
	Status status_ = Status::None;

	// フェードの持続時間
	float duration_ = 0.0f;
	// 経過時間カウンター
	float counter_ = 0.0f;
};