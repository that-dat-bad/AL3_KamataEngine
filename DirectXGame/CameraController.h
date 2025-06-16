#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

class Player;

class CameraController {
public:
	// 初期化
	void Initialize();

	// 更新
	void Update();

	void Reset();

	void SetTarget(Player* target) { target_ = target; }

	void SetCamera(Camera* camera) { camera_ = camera; }

private:
	Camera* camera_;

	Player* target_ = nullptr;

	// 追跡対象とカメラの座標の差(オフセット)
	Vector3 targetOffset_ = {0, 0, -15.0f};
};