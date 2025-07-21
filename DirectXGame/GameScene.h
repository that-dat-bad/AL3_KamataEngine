#pragma once
#include "CameraController.h"
#include "DeathParticles.h"
#include "Enemy.h"
#include "KamataEngine.h"
#include "MapChipField.h"
#include "Player.h"
#include "Skydome.h"
#include <vector>
#include"Fade.h"

class GameScene {
public:
	~GameScene();

	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	void GenerateBlocks();

	void CheckAllCollisions();

	bool IsFinished() const { return finished_; }

private:
	// テクスチャハンドル
	uint32_t textureHandle_ = 0;

	// 3Dモデルデータ
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	Camera camera_;

	// 自キャラ
	Player* player_ = nullptr;

	// 天球
	Skydome* skydome_ = nullptr;

	// デスパーティクル
	DeathParticles* deathParticles_ = nullptr;

	// 敵キャラ
	std::list<Enemy*> enemies_;
	int32_t kEnemyCount_ = 1;

	// カメラコントローラー
	CameraController* cameraController_ = nullptr;

	// 3Dモデルデータ
	Model* blockModel_ = nullptr;
	Model* playerModel_ = nullptr;
	Model* skydomeModel_ = nullptr;
	Model* enemyModel_ = nullptr;
	Model* deathParticleModel_ = nullptr;

	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	// デバックカメラ無効
	bool isDebugCameraActive_ = false;

	// デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// マップチップフィールド
	MapChipField* mapChipField_;

		// フェード
	Fade* fade_ = nullptr;

	enum class Phase {
		kFadeIn,   // フェードイン
		kPlay,    // ゲームプレイ
		kDeath,   // デス演出
		kFadeOut, // フェードアウト
	};
	// 現在のフェーズ
	Phase phase_;

	// 終了フラグ
	bool finished_ = false;

	bool isInitialized_ = false;
	                                                          
	// フェーズごとの更新処理
	void UpdatePlayPhase();
	void UpdateDeathPhase();
	// フェーズの切り替え処理
	void ChangePhase();
};
