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
	enum class ClearResult {
		kNextStage,
		kStageSelect,
		kTitle,
		kNone,
	};

	~GameScene();
	void Initialize(int stageNumber);
	void Update();
	void Draw();

	bool IsFinished() const { return finished_; }
	bool IsCleared() const { return isCleared_; }
	ClearResult GetClearResult() const { return clearResult_; }

	void CreateHitEffect(const Vector3& position, const Vector3& rotation);
	void GenerateBlocks();
	void CheckAllCollisions();

private:
	void OpenConnectedDoors(uint32_t startX, uint32_t startY);

	void UpdateFadeInPhase();
	void UpdatePlayPhase();
	void UpdateDeathPhase();
	void UpdateStageClearPhase();
	void ChangePhase();

	void InitializeStageClearUI();
	void DrawStageClearUI();

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
	Model* goalArrowModel_ = nullptr;
	WorldTransform goalArrowWorldTransform_; 
	float goalArrowAnimationTimer_ = 0.0f;
	Vector3 goalPosition_;

	std::vector<std::vector<BlockData>> blocks_;
	uint32_t textureHandleAttackFX_ = 0;
	bool isDebugCameraActive_ = false;
	DebugCamera* debugCamera_ = nullptr;
	MapChipField* mapChipField_;
	Fade* fade_ = nullptr;

	static const int kMaxKeyIcons = 3;
	Sprite* keyIcons_[kMaxKeyIcons] = {};
	Sprite* keyIconsEmpty_[kMaxKeyIcons] = {};
	Sprite* helpSprite_ = nullptr;
	enum class Phase { kFadeIn, kPlay, kDeath, kStageClear, kFadeOut };
	Phase phase_;
	bool finished_ = false;
	bool isCleared_ = false;
	bool isInitialized_ = false;
	std::list<HitEffect*> hitEffects_;


	ClearResult clearResult_ = ClearResult::kNone;
	Sprite* overlaySprite_ = nullptr;
	Sprite* buttonNextStage_ = nullptr;
	Sprite* buttonStageSelect_ = nullptr;
	Sprite* buttonBackToTitle_ = nullptr;
	Sprite* cursor_ = nullptr;
	int clearMenuSelection_ = 0;

	enum class ClearMenuPhase {
		kOverlayFadeIn, // 背景がフェードイン中
		kDelay,         // ボタン表示までの待機時間
		kActive         // 操作可能
	};
	ClearMenuPhase clearMenuPhase_;
	float clearMenuTimer_ = 0.0f;
	float overlayAlpha_ = 0.0f;
	float cursorAnimationTimer_ = 0.0f;

};