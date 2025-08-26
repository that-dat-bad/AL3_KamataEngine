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

struct BlockData {
	WorldTransform* worldTransform = nullptr;
	ObjectColor* objectColor = nullptr;
	MapChipField::MapChipType type = MapChipField::MapChipType::kBlank;
};

class GameScene {
public:
	~GameScene();
	void Initialize(int stageNumber);
	void Update();
	void Draw();
	void GenerateBlocks();
	void CheckAllCollisions();
	bool IsFinished() const { return finished_; }
	bool IsCleared() const { return isCleared_; }
	void CreateHitEffect(const Vector3& position, const Vector3& rotation);

private:
	void OpenConnectedDoors(uint32_t startX, uint32_t startY);
	void UpdateFadeInPhase();
	void UpdatePlayPhase();
	void UpdateDeathPhase();
	void ChangePhase();

	uint32_t textureHandle_ = 0;
	Model* model_ = nullptr;
	Camera camera_;
	Player* player_ = nullptr;
	Skydome* skydome_ = nullptr;
	DeathParticles* deathParticles_ = nullptr;
	std::list<Enemy*> enemies_;
	int32_t kEnemyCount_ = 1;
	CameraController* cameraController_ = nullptr;

	Model* blockModel_ = nullptr;
	Model* playerModel_ = nullptr;
	Model* skydomeModel_ = nullptr;
	Model* enemyModel_ = nullptr;
	Model* deathParticleModel_ = nullptr;
	Model* attackFxModelRight_ = nullptr;
	Model* attackFxModelLeft_ = nullptr;
	Model* hitEffectModel_ = nullptr;
	Model* lockedDoorModel_ = nullptr;
	Model* keyModel_ = nullptr;
	Model* goalModel_ = nullptr;

	std::vector<std::vector<BlockData>> blocks_;
	uint32_t textureHandleAttackFX_ = 0;
	bool isDebugCameraActive_ = false;
	DebugCamera* debugCamera_ = nullptr;
	MapChipField* mapChipField_;
	Fade* fade_ = nullptr;

	// --- ★UI用の変数を元のスプライト配列に戻します ---
	static const int kMaxKeyIcons = 3;
	Sprite* keyIcons_[kMaxKeyIcons] = {};      // 所持している鍵
	Sprite* keyIconsEmpty_[kMaxKeyIcons] = {}; // 空の鍵枠

	enum class Phase { kFadeIn, kPlay, kDeath, kFadeOut };
	Phase phase_;
	bool finished_ = false;
	bool isCleared_ = false;
	bool isInitialized_ = false;
	std::list<HitEffect*> hitEffects_;
};