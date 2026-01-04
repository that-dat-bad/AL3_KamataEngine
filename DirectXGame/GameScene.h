#pragma once
#include "Enemy.h"
#include "Explosion.h"
#include "Ground.h"
#include "IScene.h"
#include "KamataEngine.h"
#include "Player.h"
#include "Reticle.h"
#include <list>
#include <optional>
#include <vector>

struct EnemySpawnData {
	int spawnTime;
	KamataEngine::Vector3 position;
	KamataEngine::Vector3 velocity;
	std::string type;
	std::string attackPattern;
};

class GameScene : public IScene {
public:
	~GameScene() override;
	void Initialize() override;
	std::optional<SceneID> Update() override;
	void Draw() override;

private:
	std::optional<SceneID> UpdateFadeIn();
	std::optional<SceneID> UpdateMain();
	std::optional<SceneID> UpdateFadeOut();

private:
	uint32_t textureHandle_ = 0;

	// モデル
	KamataEngine::Model* playerModel_ = nullptr;
	KamataEngine::Model* playerBulletModel_ = nullptr;
	KamataEngine::Model* enemyModel_ = nullptr;
	KamataEngine::Model* enemyBulletModel_ = nullptr;
	KamataEngine::Model* playerMissileModel_ = nullptr;
	KamataEngine::Model* explosionModel_ = nullptr;
	KamataEngine::Model* groundModel_ = nullptr;

	KamataEngine::Camera camera_;
	Player* player_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Input* input_ = nullptr;
	KamataEngine::DebugCamera* debugCamera_ = nullptr;
	bool isDebugCameraActive_ = false;

	// 地面
	Ground* ground_ = nullptr;

	// リスト
	std::list<Enemy*> enemies_;
	std::list<EnemySpawnData> enemySpawnList_;
	std::list<Explosion*> explosions_;

	Reticle* reticle_ = nullptr;
	Enemy* lockedEnemy_ = nullptr;
	KamataEngine::Sprite* lockOnMark_ = nullptr;
	uint32_t lockOnTex_ = 0;

	// UI
	KamataEngine::Sprite* hpBarSprite_ = nullptr;
	KamataEngine::Sprite* lifeIconSprite_ = nullptr;
	uint32_t uiTexHandle_ = 0;
	int score_ = 0;

	// フェード
	KamataEngine::Sprite* fadeSprite_ = nullptr;
	uint32_t fadeTextureHandle_ = 0;

	int32_t gameLimitTimer_ = 0;
	int32_t gameElapsedTime_ = 0;
};