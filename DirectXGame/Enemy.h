#pragma once
#include "KamataEngine.h"
#include "mathStruct.h"
#include <numbers>

using namespace KamataEngine;

class Enemy {
public:
	void Initialize(Model* model, Camera* camera, const Vector3& position);

	void Update();

	void Draw();

private:
	static inline const float kWalkSpeed = -0.1f; // 敵の歩行速度

	Vector3 velocity_ = {}; // 敵の移動速度

	static inline const float kWalkMotionAngleStart = 0.0f; // 最初の角度

	static inline const float kWalkMotionAngleEnd = std::numbers::pi_v<float> / 2.0f; // 最後の角度

	static inline const float kWalkMotionTime = 1.0f; // 歩行モーションの周期

	// 経過時間
	float walkTimer_ = 0.0f;

	// ワールド変換データ
	WorldTransform worldTransform_;
	// 3Dモデルデータ
	Model* model_ = nullptr;

	Camera* camera_ = nullptr;
};
