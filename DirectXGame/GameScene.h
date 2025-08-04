#pragma once
#include "CameraController.h"
#include "DeathParticles.h"
#include "Enemy.h"
#include "Fade.h"
#include "HitEffect.h"
#include "KamataEngine.h"
#include "MapChipField.h"
#include "Player.h"
#include "Skydome.h"
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

	void GenerateBlocks();

	void CheckAllCollisions();

	bool IsFinished() const { return finished_; }

	void CreateHitEffect(const Vector3& position, const Vector3& rotation);

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
	Model* attackFxModelRight_ = nullptr; // 右向き用
	Model* attackFxModelLeft_ = nullptr;
	Model* hitEffectModel_ = nullptr;
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;
	uint32_t textureHandleAttackFX_ = 0;
	// デバックカメラ無効
	bool isDebugCameraActive_ = false;

	// デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// マップチップフィールド
	MapChipField* mapChipField_;

	// フェード
	Fade* fade_ = nullptr;

	enum class Phase {
		kFadeIn,  // フェードイン
		kPlay,    // ゲームプレイ
		kDeath,   // デス演出
		kFadeOut, // フェードアウト
	};
	// 現在のフェーズ
	Phase phase_;

	// 終了フラグ
	bool finished_ = false;

	bool isInitialized_ = false;

	std::list<HitEffect*> hitEffects_;

	// フェーズごとの更新処理
	void UpdateFadeInPhase();
	void UpdatePlayPhase();
	void UpdateDeathPhase();
	// フェーズの切り替え処理
	void ChangePhase();
};
