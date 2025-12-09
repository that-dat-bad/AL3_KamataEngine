#pragma once
#include "KamataEngine.h"

class TitleLogo {
private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;

	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// 動かす用の変数
	float amplitude_ = 0.5f;
	float theta_ = 0.0f;

public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update();
	void Draw();
};
