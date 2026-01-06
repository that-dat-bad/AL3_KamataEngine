#pragma once
#include "IScene.h"
#include "KamataEngine.h"
#include <vector>

class ResultScene : public IScene {
public:
	~ResultScene() override;
	void Initialize() override;
	std::optional<SceneID> Update() override;
	void Draw() override;

	// GameSceneから結果を受け取る静的変数
	static bool isWin;
	static int finalScore;

private:
	KamataEngine::Input* input_ = nullptr;

	// --- 3D背景 (天球) ---
	KamataEngine::Model* skydomeModel_ = nullptr;
	KamataEngine::WorldTransform skydomeTransform_;
	KamataEngine::Camera camera_;

	// --- フェード用 ---
	KamataEngine::Sprite* fadeSprite_ = nullptr;
	uint32_t fadeTexHandle_ = 0;
	int fadeTimer_ = 0;
	static const int kFadeDuration = 60;

	// --- 画像ハンドル ---
	uint32_t clearTexHandle_ = 0;   // text_clear.png
	uint32_t whiteTexHandle_ = 0;   // white1x1.png (スコア背景、負け演出用)
	uint32_t returnTexHandle_ = 0;  // return.png
	uint32_t numberTexHandles_[10]; // 0.png ~ 9.png

	// --- スプライト ---
	KamataEngine::Sprite* winTextSprite_ = nullptr;    // CLEAR画像
	KamataEngine::Sprite* loseResultSprite_ = nullptr; // 敗北時の赤い四角
	KamataEngine::Sprite* scoreBgSprite_ = nullptr;    // スコア背景 (白い帯)

	// スコア数字表示用スプライト配列
	std::vector<KamataEngine::Sprite*> scoreNumberSprites_;

	// 次へ進むボタン (return.png)
	KamataEngine::Sprite* nextBtnSprite_ = nullptr;
};