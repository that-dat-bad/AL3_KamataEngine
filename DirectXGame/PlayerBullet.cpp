#include "PlayerBullet.h"
#include <cassert>
#include "mathStruct.h"

using namespace KamataEngine;

void PlayerBullet::Initialize(Model* model, const Vector3& position, const Vector3& velocity) {
	assert(model);
	model_ = model;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	velocity_ = velocity;
	deathTimer_ = kLifeTime;
	UpdateWorldMatrix(worldTransform_);
}

void PlayerBullet::Update() {
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}
	worldTransform_.translation_ += velocity_;

	UpdateWorldMatrix(worldTransform_);
}

void PlayerBullet::Draw(const Camera& camera) { model_->Draw(worldTransform_, camera); }

void PlayerBullet::OnCollision() { isDead_ = true; }