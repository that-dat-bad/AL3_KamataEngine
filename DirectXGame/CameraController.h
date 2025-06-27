#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

class Player;

class CameraController {
public:
	struct Rect {
		float left = 0.0f;
		float right = 1.0f;
		float bottom = 0.0f;
		float top = 1.0f;
	};

	// 初期化
	void Initialize();

	// 更新
	void Update();

	void Reset();

	void SetTarget(Player* target) { target_ = target; }

	void SetCamera(Camera* camera) { camera_ = camera; }

	void SetMovableCamera(Rect area) { movableArea_ = area; };

private:
	Camera* camera_;

	Player* target_ = nullptr;

	// 追跡対象とカメラの座標の差(オフセット)
	Vector3 targetOffset_ = {0, 0, -50.0f};

	Rect movableArea_ = {0.0f, 100, 0.0f, 100}; // 移動可能領域の矩形

	//カメラの目標座標
	Vector3 targetPosition_;

	//座標補間割合
	static inline const float kInterpolationRate = 0.1f;

	static inline const float kVelocityBias = 2; // カメラの座標補間における速度のバイアス

	static inline const Rect kMargin = {4.0f,4.0f, 4.0f, 4.0f}; // カメラの移動可能領域のマージン
};