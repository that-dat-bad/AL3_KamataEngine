#pragma once
#include "KamataEngine.h"

class Reticle {
public:
	// 初期化
	void Initialize();

	// 更新
	void Update(const KamataEngine::Vector3& targetWorldPos, const KamataEngine::Camera& camera);

	// 描画
	void Draw();

	KamataEngine::Vector2 GetPosition() const { return position_; }

private:
	// スプライト
	KamataEngine::Sprite* sprite_ = nullptr;
	uint32_t textureHandle_ = 0;

	// 画面上の位置
	KamataEngine::Vector2 position_ = {0, 0};
};