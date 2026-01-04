#include "Ground.h"
#include "mathStruct.h"

using namespace KamataEngine;

void Ground::Initialize(Model* model) {
	model_ = model;

	for (int i = 0; i < kGroundCount; i++) {
		worldTransforms_[i].Initialize();

		// 巨大化させる (幅100倍, 奥行き40倍)
		worldTransforms_[i].scale_ = {100.0f, 1.0f, 40.0f};

		// 自機より少し下に配置
		worldTransforms_[i].translation_.y = -10.0f;

		// 奥に向かってズラして並べる (0, 40, 80...)
		worldTransforms_[i].translation_.z = i * kGroundDepth;
	}
}

void Ground::Update() {
	// スクロール速度 (自機のスピード感)
	const float kScrollSpeed = 1.0f;

	for (int i = 0; i < kGroundCount; i++) {
		// 手前(Zマイナス方向)に移動
		worldTransforms_[i].translation_.z -= kScrollSpeed;

		// カメラの後ろ（ある程度手前）まで来たら、一番奥にリサイクル
		// 基準: -kGroundDepth (1枚分通り過ぎたら)
		if (worldTransforms_[i].translation_.z <= -kGroundDepth) {

			// ズレを補正して一番奥へ
			// (現在の位置 + 全体の長さ)
			worldTransforms_[i].translation_.z += kGroundCount * kGroundDepth;
		}

		// 行列更新
		UpdateWorldMatrix(worldTransforms_[i]);
	}
}

void Ground::Draw(const Camera& camera) {
	for (int i = 0; i < kGroundCount; i++) {
		model_->Draw(worldTransforms_[i], camera);
	}
}