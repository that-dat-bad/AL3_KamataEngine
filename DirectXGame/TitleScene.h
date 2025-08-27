#pragma once
#include "Fade.h"
#include "KamataEngine.h"

using namespace KamataEngine;

class TitleScene {
public:
	~TitleScene();
	void Initialize();
	void Update();
	void Draw();
	bool IsFinished() const { return finished_; }
	int GetSelectedStage() const { return selectedStage_; }

private:
	enum class Phase {
		kFadeIn,
		kMain,
		kFadeOut,
	};
	Phase phase_;
	Fade* fade_ = nullptr;
	Sprite* titleImage_ = nullptr;
	Sprite* backgroundSprite_ = nullptr;

	static inline const float kFadeDuration = 1.0f;

	bool finished_ = false;
	int selectedStage_ = 0; // 0:未選択, 1:ステージ1へ, -1:ステージセレクトへ
};