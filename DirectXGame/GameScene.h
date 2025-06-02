#pragma once
#include "KamataEngine.h"
#include "Player.h"
#include <vector>

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
	// テクスチャハンドル
	uint32_t textureHandle_ = 0;

	// 3Dモデルデータ
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	Camera camera_;

	// 自キャラ
	Player* player_ = nullptr;

	// 3Dモデルデータ
	Model* blockModel_ = nullptr;

	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;


	// デバックカメラ無効
	bool isDebugCameraActive_ = false;

	// デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;
};
