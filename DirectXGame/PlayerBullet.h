#pragma once
#include "KamataEngine.h"

class PlayerBullet {
public:
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity);
	void Update();
	void Draw(const KamataEngine::Camera& camera);

	bool IsDead() const { return isDead_; }
	void OnCollision();

	KamataEngine::Vector3 GetWorldPosition() const { return worldTransform_.translation_; }

	//攻撃力
	int GetPower() const { return 1; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Vector3 velocity_;
	bool isDead_ = false;

	static const int32_t kLifeTime = 60 * 5;
	int32_t deathTimer_ = kLifeTime;
};