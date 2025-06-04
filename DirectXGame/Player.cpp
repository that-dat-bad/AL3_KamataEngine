#include "Player.h"
#include "mathStruct.h"
#include <algorithm>
#include <numbers>

void Player::Initialize(Model* model, Camera* camera, const Vector3& position) {
	// nullポインタチェック
	assert(model);

	// 引数として受け取ったデータをメンバ変数に記録する
	model_ = model;
	camera_ = camera;
	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
}

void Player::Update() {
	// 移動入力
	if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {

		// 左右加速
		Vector3 acceleration = {};
		if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
			if (velocity_.x < 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}
			acceleration.x += kAcceleration;
			if (lrdirection_!=LRDirection::kRight) {
				lrdirection_ = LRDirection::kRight;
			}
		} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
			if (velocity_.x > 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}
			acceleration.x -= kAcceleration;
			if (lrdirection_ != LRDirection::kLeft) {
				lrdirection_ = LRDirection::kLeft;
			}
		}

		velocity_.x += acceleration.x;
		velocity_.y += acceleration.y;
		velocity_.z += acceleration.z;
		// 最大速度制限
		velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
	} else {
		velocity_.x *= (1.0f - kAttenuation);
	}
	// 移動
	worldTransform_.translation_.x += velocity_.x;
	worldTransform_.translation_.y += velocity_.y;
	worldTransform_.translation_.z += velocity_.z;

	// 行列更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	// 行列を定数バッファに転送
	worldTransform_.TransferMatrix();
}

void Player::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Model::PreDraw(dxCommon->GetCommandList());
	model_->Draw(worldTransform_, *camera_);
	Model::PostDraw();
}
