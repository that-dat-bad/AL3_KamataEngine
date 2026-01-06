#pragma once
#include "KamataEngine.h"

class Player;

class EnemyMissile {
public:
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position, Player* target);
	void Update();
	void Draw(const KamataEngine::Camera& camera);

	bool IsDead() const { return isDead_; }
	void OnCollision();

	KamataEngine::Vector3 GetWorldPosition() const { return worldTransform_.translation_; }

	int GetPower() const { return 10; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;

	// ★変更: ターゲット
	Player* target_ = nullptr;
	KamataEngine::Vector3 velocity_;

	bool isDead_ = false;
	int32_t deathTimer_ = 0;
};