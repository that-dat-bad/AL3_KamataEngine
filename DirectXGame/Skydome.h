#pragma once
#include "KamataEngine.h"
#include <stdint.h>

using namespace KamataEngine;

class Skydome {
public:
	void Initialize(KamataEngine::Model* model, uint32_t textureHandle, KamataEngine::Camera* camera);

	void Update();

	void Draw();

private:
	// ワールド変換データ
	WorldTransform worldTransform_;
	// 3Dモデルデータ
	Model* model_ = nullptr;
	// テクスチャハンドル
	uint32_t textureHandle_ = 0;

	Camera* camera_ = nullptr;
};
