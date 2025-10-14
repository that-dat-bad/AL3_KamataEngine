#pragma once
#include "KamataEngine.h"
#include "mathStruct.h"
#include <cassert>

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

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	Camera* camera_ = nullptr;
	bool isDead_ = false;
};