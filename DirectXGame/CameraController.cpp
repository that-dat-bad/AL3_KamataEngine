#include "CameraController.h"
#include "Player.h"
#include "mathStruct.h"

void CameraController::Initialize() {
	// カメラの初期化
	camera_->Initialize();
}

void CameraController::Update() {
	// 追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
	Vector3 targetVelocity = target_->GetVelocity();
	// 追従対象とオフセットと追従対象の速度からカメラの目標座標を計算
	targetPosition_ = targetWorldTransform.translation_ + targetOffset_ + Vector3{targetVelocity.x * kVelocityBias, targetVelocity.y * kVelocityBias, targetVelocity.z * kVelocityBias};

	camera_->translation_ = Lerp(camera_->translation_, targetPosition_, kInterpolationRate);

	//camera_->translation_.x = (std::max)(camera_->translation_.x, target_->GetWorldTransform().translation_.x + kMargin.left);
	//camera_->translation_.x = (std::min)(camera_->translation_.x, target_->GetWorldTransform().translation_.x + kMargin.right);
	//camera_->translation_.y = (std::max)(camera_->translation_.y, target_->GetWorldTransform().translation_.y + kMargin.bottom);
	//camera_->translation_.y = (std::min)(camera_->translation_.y, target_->GetWorldTransform().translation_.y + kMargin.top);

	camera_->translation_.x = targetWorldTransform.translation_.x;
	camera_->translation_.y = (std::max)(camera_->translation_.y, target_->GetWorldTransform().translation_.y + kMargin.bottom);
	camera_->translation_.y = (std::min)(camera_->translation_.y, target_->GetWorldTransform().translation_.y + kMargin.top);

	// 追従対象とオフセットからカメラの座標を計算
	camera_->translation_.x = (std::max)(camera_->translation_.x, movableArea_.left);
	camera_->translation_.x = (std::min)(camera_->translation_.x, movableArea_.right);
	camera_->translation_.y = (std::max)(camera_->translation_.y, movableArea_.bottom);
	camera_->translation_.y = (std::min)(camera_->translation_.y, movableArea_.top);

	// 行列の更新
	camera_->UpdateMatrix();
}

void CameraController::Reset() {
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
	camera_->translation_ = targetWorldTransform.translation_ + targetOffset_;
	camera_->UpdateMatrix();
}
