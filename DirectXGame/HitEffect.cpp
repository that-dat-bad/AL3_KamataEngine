#include "HitEffect.h"
#include "mathStruct.h"

// --- 静的メンバ変数の実体を定義 ---
Model* HitEffect::model_ = nullptr;
Camera* HitEffect::camera_ = nullptr;

HitEffect* HitEffect::Create(const Vector3& position, const Vector3& rotation) {
	// インスタンスを生成
	HitEffect* instance = new HitEffect();
	// newの失敗を検出
	assert(instance);
	// インスタンスの初期化
	instance->Initialize(position, rotation);
	// 初期化したインスタンスを返す
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
	circleWorldTransform_.translation_.z -= 2.5f;

	// 尖った筋エフェクトの初期化
	for (WorldTransform& worldTransform : streakWorldTransforms_) {
		worldTransform.Initialize();
		worldTransform.rotation_ = circleWorldTransform_.rotation_;
		worldTransform.rotation_.x += rotationDistribution(randomEngine_);
		worldTransform.translation_ = position;
		worldTransform.translation_.z -= 2.5f;
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
	case State::kSpread: {
		float t = static_cast<float>(counter_) / kSpreadDuration;
		// 円は広がる
		circleWorldTransform_.scale_ = {EaseOut(0.0f, 1.0f, t), EaseOut(0.0f, 1.0f, t), 1.0f};
		// 筋は伸びてから細くなる
		for (WorldTransform& worldTransform : streakWorldTransforms_) {
			worldTransform.scale_.x = EaseOut(0.0f, 100.0f, t); // 長さ
			worldTransform.scale_.y = EaseIn(10.0f, 0.0f, t);  // 太さ
		}
		if (counter_ >= kSpreadDuration) {
			state_ = State::kFade;
			counter_ = 0;
		}
	} break;
	case State::kFade: {
		float t = static_cast<float>(counter_) / kFadeDuration;
		float alpha = EaseOut(1.0f, 0.0f, t);
		objectColor_.SetColor({1, 1, 1, alpha});
		if (counter_ >= kFadeDuration) {
			state_ = State::kDead;
		}
	} break;
	}

	// 行列更新
	circleWorldTransform_.matWorld_ = MakeAffineMatrix(circleWorldTransform_.scale_, circleWorldTransform_.rotation_, circleWorldTransform_.translation_);
	circleWorldTransform_.TransferMatrix();
	for (WorldTransform& worldTransform : streakWorldTransforms_) {
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
	// 筋を描画
	for (WorldTransform& worldTransform : streakWorldTransforms_) {
		model_->Draw(worldTransform, *camera_, &objectColor_);
	}
	Model::PostDraw();
}