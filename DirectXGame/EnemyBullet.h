#pragma once
#include "KamataEngine.h"
#include <stdint.h>

class EnemyBullet {
public:
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity);
	void Update();
	void Draw(const KamataEngine::Camera& camera);
	bool IsDead() const { return isDead_; }

	// ★追加: 座標取得
	KamataEngine::Vector3 GetWorldPosition() const { return worldTransform_.translation_; }
	// ★追加: 衝突通知
	void OnCollision() { isDead_ = true; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	uint32_t textureHandle_ = 0;
	KamataEngine::Vector3 velocity_;
	static const int32_t kLifeTime = 60 * 5;
	int32_t deathTimer_ = kLifeTime;
	bool isDead_ = false;
};