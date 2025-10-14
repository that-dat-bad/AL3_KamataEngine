#pragma once
#include "Collision.h"
#include "EnemyManager.h"
#include "KamataEngine.h"
#include "mathStruct.h"
#include <cassert>

using namespace KamataEngine;

class EnemyManager;

class Bullet {
public:
	// 初期化
	void Initialize(Model* model, Camera* camera, const Vector3& position, const Vector3& velocity, EnemyManager* enemyManager);
	// 更新
	void Update();
	// 描画
	void Draw();

	// 生存フラグ
	bool IsDead() const { return isDead_; }

	AABB GetAABB();

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	Camera* camera_ = nullptr;
	Vector3 velocity_ = {};

	// 寿命
	static inline const int32_t kLifeTime = 120;
	int32_t lifeTimer_ = kLifeTime;
	bool isDead_ = false;

	static inline const Vector3 kBulletSize = {0.2f, 0.2f, 0.2f};

	EnemyManager* enemyManager_ = nullptr;
};