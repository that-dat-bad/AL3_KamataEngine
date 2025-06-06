#pragma once
#include "KamataEngine.h"
#include <stdint.h>
using namespace KamataEngine;

class Player {
public:
	// 初期化
	void Initialize(Model* model,Camera* camera,const Vector3& position);
	// 更新
	void Update();
	// 描画
	void Draw();

private:
	// ワールド変換データ
	WorldTransform worldTransform_;
	// 3Dモデルデータ
	Model* model_ = nullptr;


	Camera* camera_ = nullptr;

	Vector3 velocity_ = {};

	static inline const float kAcceleration = 0.01f;
	static inline const float kAttenuation = 0.02f;
	static inline const float kLimitRunSpeed = 0.3f;
	static inline const float kTimeTurn = 0.3f;
	static inline const float kGravityAcceleration = 0.005f;
	static inline const float kLimitFallSpeed = 0.3f;
	static inline const float kJumpAcceleration = 0.2f;

	enum class LRDirection {
		kRight,
		kLeft,
	};
	LRDirection lrdirection_ = LRDirection::kRight;

	//旋回開始時の角度
	float turnFIrstRotationY_ = 0.0f;

	//旋回タイマー
	float turnTimer_ = 0.0f;

	//接地判定
	bool onGround_ = true;
};
