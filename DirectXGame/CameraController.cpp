#include "CameraController.h"
#include "Player.h"

void CameraController::Initialize() {
	// カメラの初期化
	camera_->Initialize();
}

void CameraController::Update() {
	//追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
	//追従対象とオフセットからカメラの座標を計算
	camera_->translation_ = targetWorldTransform.translation_ + targetOffset_;
	//行列の更新
	camera_->UpdateMatrix();
}

void CameraController::Reset() {
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
	camera_->translation_ = targetWorldTransform.translation_ + targetOffset_;
	camera_->UpdateMatrix();
}
