#pragma once
#include "KamataEngine.h"

class Explosion {
public:
	// 初期化（モデルと発生場所を受け取る）
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position);

	// 更新（膨らんでいく）
	void Update();

	// 描画
	void Draw(const KamataEngine::Camera& camera);

	// 死んだかチェック
	bool IsDead() const { return isDead_; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;

	bool isDead_ = false;
	int32_t timer_ = 0;

	// 爆発の最大寿命
	static const int32_t kLifeTime = 20;
};