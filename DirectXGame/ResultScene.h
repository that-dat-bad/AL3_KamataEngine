// ResultScene.h
#pragma once
#include "IScene.h"
#include "KamataEngine.h"

// ISceneを継承する
class ResultScene : public IScene {
public:
	~ResultScene() override;

	// 必要な関数をオーバーライド
	void Initialize() override;
	std::optional<SceneID> Update() override;
	void Draw() override;

	// ★追加: 勝敗判定フラグ (trueなら勝ち、falseなら負け)
	// staticにすることで、GameSceneから直接 ResultScene::isWin = true; と書き込めるようにする
	static bool isWin;

private:
	// テクスチャハンドル
	uint32_t bgTex_ = 0;
	uint32_t winTex_ = 0;
	uint32_t loseTex_ = 0;
	uint32_t rtbTex_ = 0;

	// スプライト
	KamataEngine::Sprite* bgSprite_ = nullptr;     // 背景用
	KamataEngine::Sprite* resultSprite_ = nullptr; // youWin または youLose
	KamataEngine::Sprite* rtbSprite_ = nullptr;    // RTB (Return To Base)

	// 点滅用タイマー
	float blinkTimer_ = 0.0f;
};