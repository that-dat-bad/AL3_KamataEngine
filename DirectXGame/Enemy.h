#pragma once
#include "KamataEngine.h"
#include "mathStruct.h"
#include <cassert>
#include "Collision.h"

using namespace KamataEngine;

class Enemy {
public:



	// 初期化
	void Initialize(Model* model, Camera* camera, const Vector3& position);
	// 更新
	void Update();
	// 描画
	void Draw();

	bool IsDead() const { return isDead_; }
	void OnCollision() { isDead_ = true; }
	Vector3 GetWorldPosition();
	AABB GetAABB() {
		AABB aabb;
		Vector3 worldPos = GetWorldPosition();
		aabb.min = worldPos - kEnemySize * 0.5f;
		aabb.max = worldPos + kEnemySize * 0.5f;
		return aabb;
	}

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	Camera* camera_ = nullptr;
	bool isDead_ = false;

	// 敵のサイズ
	static inline const Vector3 kEnemySize = {1.8f, 1.8f, 1.8f};
};