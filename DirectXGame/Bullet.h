#pragma once
#include "KamataEngine.h"
#include "mathStruct.h"
#include <cassert>

using namespace KamataEngine;

class Bullet {
public:
	// 初期化
	void Initialize(Model* model, Camera* camera, const Vector3& position, const Vector3& velocity);
	// 更新
	void Update();
	// 描画
	void Draw();

	// 生存フラグ
	bool IsDead() const { return isDead_; }

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	Camera* camera_ = nullptr;
	Vector3 velocity_ = {};

	// 寿命
	static inline const int32_t kLifeTime = 120;
	int32_t lifeTimer_ = kLifeTime;
	bool isDead_ = false;
};