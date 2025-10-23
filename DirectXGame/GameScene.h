#pragma once
#include "BulletManager.h"
#include "CameraController.h"
#include "EnemyManager.h"
#include "KamataEngine.h"
#include "MapChipField.h"
#include "Player.h"
#include "TitleScene.h"
#include <vector>

class GameScene {
public:
	~GameScene();

	// 初期化
	void Initialize(int stageIndex);

	// 更新
	SceneManager Update();

	// 描画
	void Draw();

	void GenerateBlocks();

	static void SetSelectedStageIndex(int index) { selectedStageIndex_ = index; }

	static int selectedStageIndex_;

private:
	// テクスチャハンドル
	uint32_t textureHandle_ = 0;

	// カメラ
	Camera camera_;

	// 自キャラ
	Player* player_ = nullptr;


	// カメラコントローラー
	CameraController* cameraController_ = nullptr;

	// 3Dモデルデータ
	Model* blockModel_ = nullptr;
	Model* playerModel_ = nullptr;
	Model* bulletModel_ = nullptr;
	Model* enemyModel_ = nullptr;
	Model* arrowModel_ = nullptr;

	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	// デバックカメラ無効
	bool isDebugCameraActive_ = false;

	// デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// マップチップフィールド
	MapChipField* mapChipField_;

	BulletManager* bulletManager_ = nullptr;
	EnemyManager* enemyManager_ = nullptr;
};