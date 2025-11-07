#pragma once
#include "KamataEngine.h"
#include "Player.h"

class GameScene {
public:
	~GameScene();

	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

private:
	uint32_t textureHandle_ = 0;

	// 3Dモデルデータ
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera camera_;

	// 自キャラ
	Player* player_ = nullptr;

	KamataEngine::WorldTransform worldTransform_;

	// キーボード入力
	KamataEngine::Input* input_ = nullptr;

	// デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// デバッグカメラ有効
	bool isDebugCameraActive_ = false;
};