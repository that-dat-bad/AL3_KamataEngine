#pragma once
#include "KamataEngine.h"

class Ground {
public:
	// 引数を1つ（地面モデルのみ）に戻します
	void Initialize(KamataEngine::Model* model);

	void Update();

	void Draw(const KamataEngine::Camera& camera);

private:
	KamataEngine::Model* model_ = nullptr;

	// 地面を隙間なく並べるための枚数
	static const int kGroundCount = 20;
	KamataEngine::WorldTransform worldTransforms_[kGroundCount];

	// 地面1枚の奥行きの長さ
	const float kGroundDepth = 200.0f;
};