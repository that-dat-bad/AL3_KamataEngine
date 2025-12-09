#pragma once
#include "KamataEngine.h"

class TitleGuide {
private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;

	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// ★追加：点滅制御用のパラメータ
	float blinkParameter_ = 0.0f;

	// ★追加：点滅の周期（速さ）
	const float kBlinkSpeed_ = 0.1f;

public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update();
	void Draw();
	void SetPosition(const KamataEngine::Vector3& position) { worldTransform_.translation_ = position; }
};