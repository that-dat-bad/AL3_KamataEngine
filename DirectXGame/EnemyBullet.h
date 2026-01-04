#pragma once
#include "KamataEngine.h"

class Player; // 前方宣言

class EnemyBullet {
public:
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity, bool isHoming = false, Player* target = nullptr);

	void Update();
	void Draw(const KamataEngine::Camera& camera);

	bool IsDead() const { return isDead_; }
	void OnCollision() { isDead_ = true; }
	KamataEngine::Vector3 GetWorldPosition() const { return worldTransform_.translation_; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Vector3 velocity_;

	bool isDead_ = false;
	int32_t deathTimer_ = 0;
	static const int32_t kLifeTime = 60 * 5;

	//追尾用
	bool isHoming_ = false;
	Player* target_ = nullptr;
};