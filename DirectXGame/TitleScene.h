#pragma once
#include "IScene.h"
#include "KamataEngine.h"
#include "TitleLogo.h"
#include <list>

class TitleScene : public IScene {
public:
	~TitleScene();

	// 初期化
	void Initialize() override;

	// 更新
	std::optional<SceneID> Update() override;

	// 描画
	void Draw() override;

private:
	// --- フェード用 ---
	uint32_t textureHandle_ = 0;
	KamataEngine::Sprite* fadeSprite_ = nullptr;
	uint32_t fadeTextureHandle_ = 0;

	// --- 3Dモデルデータ ---
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera camera_;
	KamataEngine::WorldTransform worldTransform_;

	// --- 入力 ---
	KamataEngine::Input* input_ = nullptr;

	// --- デバッグカメラ ---
	KamataEngine::DebugCamera* debugCamera_ = nullptr;
	bool isDebugCameraActive_ = false;

	// --- フェーズ管理 ---
	std::optional<SceneID> UpdateFadeIn();
	std::optional<SceneID> UpdateMain();
	std::optional<SceneID> UpdateFadeOut();

	// --- タイトルロゴ (3D) ---
	TitleLogo* logo_ = nullptr;
	KamataEngine::Vector3 logoPosition_ = {0.0f, 0.0f, -45.0f};

	// --- 背景 (天球) ---
	KamataEngine::Model* skydomeModel_ = nullptr;
	KamataEngine::WorldTransform skydomeTransform_;

	// --- 誘導表示 (スプライト) ---
	KamataEngine::Sprite* pressSpaceSprite_ = nullptr; // スプライト本体
	uint32_t pressSpaceTexture_ = 0;                   // テクスチャハンドル
	float blinkTimer_ = 0.0f;                          // 点滅用タイマー
};