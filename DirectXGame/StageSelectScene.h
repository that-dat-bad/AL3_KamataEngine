#pragma once
#include "Fade.h"
#include "KamataEngine.h"

using namespace KamataEngine;

class StageSelectScene {
public:
	~StageSelectScene();
	void Initialize();
	void Update();
	void Draw();

	bool IsFinished() const { return isFinished_; }
	int GetSelectedStage() const { return selectedStage_; }

private:
	static const int kMaxStages = 5;
	int selectedStage_ = 1;
	bool isFinished_ = false;

	Sprite* sprites_[kMaxStages] = {};
	Sprite* background_ = nullptr;

	Fade* fade_ = nullptr;
	enum class Phase { kFadeIn, kMain, kFadeOut };
	Phase phase_ = Phase::kFadeIn;

	float animationTimer_ = 0.0f;

	Vector2 originalSpriteSizes_[kMaxStages];
};