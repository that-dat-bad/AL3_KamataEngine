#include "EnemyBullet.h"
#include "Player.h"
#include "mathStruct.h"
#include <cassert>
#include <cmath>

using namespace KamataEngine;

void EnemyBullet::Initialize(Model* model, const Vector3& position, const Vector3& velocity, bool isHoming, Player* target) {
	assert(model);
	model_ = model;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	velocity_ = velocity;

	isHoming_ = isHoming;
	target_ = target;

	deathTimer_ = kLifeTime;
}

void EnemyBullet::Update() {
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}

	if (isHoming_ && target_) {
		Vector3 targetPos = target_->GetWorldPosition();
		Vector3 toTarget = {targetPos.x - worldTransform_.translation_.x, targetPos.y - worldTransform_.translation_.y, targetPos.z - worldTransform_.translation_.z};

		float dist = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y + toTarget.z * toTarget.z);
		if (dist > 0) {
			toTarget.x /= dist;
			toTarget.y /= dist;
			toTarget.z /= dist;
		}

		const float kHomingStrength = 0.05f;
		velocity_.x = LerpShort(velocity_.x, toTarget.x * 0.5f, kHomingStrength);
		velocity_.y = LerpShort(velocity_.y, toTarget.y * 0.5f, kHomingStrength);
		velocity_.z = LerpShort(velocity_.z, toTarget.z * 0.5f, kHomingStrength);
	}

	worldTransform_.translation_ += velocity_;

	UpdateWorldMatrix(worldTransform_);
}

void EnemyBullet::Draw(const Camera& camera) { model_->Draw(worldTransform_, camera); }