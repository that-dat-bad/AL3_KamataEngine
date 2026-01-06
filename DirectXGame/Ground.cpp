#include "Ground.h"
#include "mathStruct.h"

using namespace KamataEngine;

void Ground::Initialize(Model* model) {
	model_ = model;

	for (int i = 0; i < kGroundCount; i++) {
		worldTransforms_[i].Initialize();

		// 地面のスケール（必要なら調整してください）
		worldTransforms_[i].scale_ = {1.0f, 1.0f, 1.0f};

		// 自機より少し下に配置
		worldTransforms_[i].translation_.y = -10.0f;

		// 奥に向かってズラして並べる
		worldTransforms_[i].translation_.z = i * kGroundDepth;

		UpdateWorldMatrix(worldTransforms_[i]);
	}
}

void Ground::Update() {
	// スクロール速度
	const float kScrollSpeed = 3.0f;

	for (int i = 0; i < kGroundCount; i++) {
		// 手前(Zマイナス方向)に移動
		worldTransforms_[i].translation_.z -= kScrollSpeed;

		// カメラの後ろ（ある程度手前）まで来たら、一番奥にリサイクル
		// 基準: -kGroundDepth (1枚分通り過ぎたら)
		if (worldTransforms_[i].translation_.z <= -kGroundDepth) {
			// ズレを補正して一番奥へ
			worldTransforms_[i].translation_.z += kGroundCount * kGroundDepth;
		}

		// 行列更新
		UpdateWorldMatrix(worldTransforms_[i]);
	}
}

void Ground::Draw(const Camera& camera) {
	if (model_) {
		for (int i = 0; i < kGroundCount; i++) {
			model_->Draw(worldTransforms_[i], camera);
		}
	}
}