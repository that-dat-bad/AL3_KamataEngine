// HitEffect.cpp
#include "HitEffect.h"
#include "mathStruct.h"

// --- 静的メンバ変数の実体を定義 ---
Model* HitEffect::model_ = nullptr;
Camera* HitEffect::camera_ = nullptr;

HitEffect* HitEffect::Create(const Vector3& position, const Vector3& rotation) {
	HitEffect* instance = new HitEffect();
	assert(instance);
	instance->Initialize(position, rotation);
	return instance;
}

void HitEffect::Initialize(const Vector3& position, const Vector3& rotation) {
	// 乱数生成エンジンの初期化
	std::random_device seedGenerator;
	randomEngine_.seed(seedGenerator());

	// 乱数範囲の初期化 (-πからπまで)
	std::uniform_real_distribution<float> rotationDistribution(-3.14159f, 3.14159f);

	// 円エフェクトの初期化
	circleWorldTransform_.Initialize();
	circleWorldTransform_.translation_ = position;
	circleWorldTransform_.rotation_ = rotation;
	circleWorldTransform_.rotation_.y += 3.14159f;
	circleWorldTransform_.translation_.z -= 2.2f;

	// 楕円エフェクトの初期化
	for (WorldTransform& worldTransform : ellipseWorldTransforms_) {
		worldTransform.Initialize();
		worldTransform.scale_ = {1.5f, 0.5f, 1.0f};                                   // 楕円の長さを設定
		worldTransform.rotation_ = {0.0f, 0.0f, rotationDistribution(randomEngine_)}; // Z軸でランダムに回転
		worldTransform.translation_ = position;
		worldTransform.translation_.z -= 0.2f;
	}

	objectColor_.Initialize();
	state_ = State::kSpread;
	counter_ = 0;
}

void HitEffect::Update() {
	if (state_ == State::kDead) {
		return;
	}

	counter_++;

	switch (state_) {
	case State::kSpread: // 広がる処理
	{
		float t = static_cast<float>(counter_) / kSpreadDuration;
		// 円と楕円のスケールをアニメーション
		circleWorldTransform_.scale_ = {EaseOut(0.0f, 1.0f, t), EaseOut(0.0f, 1.0f, t), 1.0f};
		for (WorldTransform& worldTransform : ellipseWorldTransforms_) {
			worldTransform.scale_.x = EaseOut(0.0f, 1.5f, t);
			worldTransform.scale_.y = EaseOut(0.0f, 0.5f, t);
		}
		if (counter_ >= kSpreadDuration) {
			state_ = State::kFade;
			counter_ = 0; // カウンターリセット
		}
	} break;
	case State::kFade: // 消える処理
	{
		float t = static_cast<float>(counter_) / kFadeDuration;
		float alpha = EaseOut(1.0f, 0.0f, t);
		objectColor_.SetColor({1, 1, 1, alpha});
		if (counter_ >= kFadeDuration) {
			state_ = State::kDead; // 死亡状態へ
		}
	} break;
	}

	// 全オブジェクトの行列を更新
	circleWorldTransform_.matWorld_ = MakeAffineMatrix(circleWorldTransform_.scale_, circleWorldTransform_.rotation_, circleWorldTransform_.translation_);
	circleWorldTransform_.TransferMatrix();
	for (WorldTransform& worldTransform : ellipseWorldTransforms_) {
		worldTransform.matWorld_ = MakeAffineMatrix(worldTransform.scale_, worldTransform.rotation_, worldTransform.translation_);
		worldTransform.TransferMatrix();
	}
}

void HitEffect::Draw() {
	if (state_ == State::kDead) {
		return;
	}
	Model::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	// 円を描画
	model_->Draw(circleWorldTransform_, *camera_, &objectColor_);
	// 楕円を描画
	for (WorldTransform& worldTransform : ellipseWorldTransforms_) {
		model_->Draw(worldTransform, *camera_, &objectColor_);
	}
	Model::PostDraw();
}