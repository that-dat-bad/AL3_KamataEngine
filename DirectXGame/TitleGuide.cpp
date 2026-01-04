#include "TitleGuide.h"
#include "KamataEngine.h"
#include <cassert>
#include <cmath>
#include"mathStruct.h"
using namespace KamataEngine;

void TitleGuide::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
	// nullポインタチェック
	assert(model);

	// 引数をメンバ変数に記録
	model_ = model;
	camera_ = camera;

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	// 点滅用パラメータの初期化
	blinkParameter_ = 0.0f;
}

void TitleGuide::Update() {
	// 時間を進める
	blinkParameter_ += kBlinkSpeed_;

	float alpha = (std::sin(blinkParameter_) + 1.0f) / 2.0f;


	model_->SetAlpha(alpha);

	// 行列の更新処理（今まで通り）
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}


void TitleGuide::Draw() {
	model_->Draw(worldTransform_, *camera_);
}