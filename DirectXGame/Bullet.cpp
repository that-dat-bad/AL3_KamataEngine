#include "Bullet.h"

void Bullet::Initialize(Model* model, Camera* camera, const Vector3& position, const Vector3& velocity) {
	assert(model);
	assert(camera);

	model_ = model;
	camera_ = camera;
	velocity_ = velocity;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
}

void Bullet::Update() {
	// 寿命が尽きたらフラグを立てる
	lifeTimer_--;
	if (lifeTimer_ <= 0) {
		isDead_ = true;
	}

	worldTransform_.translation_ += velocity_;

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Bullet::Draw() {
	model_->Draw(worldTransform_, *camera_);
}