#pragma once
#include "KamataEngine.h"

class Enemy;

class PlayerMissile {
public:
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position, Enemy* target);
	void Update();
	void Draw(const KamataEngine::Camera& camera);

	bool IsDead() const { return isDead_; }
	void OnCollision();

	KamataEngine::Vector3 GetWorldPosition() const { return worldTransform_.translation_; }

	//攻撃力
	int GetPower() const { return 10; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	Enemy* target_ = nullptr; // 追尾対象
	KamataEngine::Vector3 velocity_;

	bool isDead_ = false;
	int32_t deathTimer_ = 0;
};