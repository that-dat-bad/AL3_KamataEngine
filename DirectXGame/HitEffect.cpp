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
	// 円エフェクトのワールドトランスフォームを発生座標で初期化
	circleWorldTransform_.Initialize();
	circleWorldTransform_.translation_ = position;
	// プレイヤーの向きを反映
	circleWorldTransform_.rotation_ = rotation;
	// 平面をカメラに向けるためにX軸で90度回転させる
	circleWorldTransform_.rotation_.x += 1.5708f; // (π / 2)
}

void HitEffect::Update() {
	circleWorldTransform_.matWorld_ = MakeAffineMatrix(circleWorldTransform_.scale_, circleWorldTransform_.rotation_, circleWorldTransform_.translation_);
	circleWorldTransform_.TransferMatrix();
}

void HitEffect::Draw() {
	Model::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	model_->Draw(circleWorldTransform_, *camera_);
	Model::PostDraw();
}