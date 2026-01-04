#include "PlayerMissile.h"
#include "Enemy.h"
#include "mathStruct.h"
#include <cassert>
#include <cmath>

using namespace KamataEngine;

void PlayerMissile::Initialize(Model* model, const Vector3& position, Enemy* target) {
	assert(model);
	model_ = model;
	target_ = target;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {2.0f, 2.0f, 2.0f};

	velocity_ = {0.0f, 0.0f, 1.0f};
	deathTimer_ = 60 * 10;
}

void PlayerMissile::Update() {
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}

	if (target_ && !target_->IsDead()) {
		Vector3 targetPos = target_->GetWorldPosition();
		Vector3 toTarget = {targetPos.x - worldTransform_.translation_.x, targetPos.y - worldTransform_.translation_.y, targetPos.z - worldTransform_.translation_.z};

		float dist = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y + toTarget.z * toTarget.z);
		if (dist > 0.0f) {
			toTarget.x /= dist;
			toTarget.y /= dist;
			toTarget.z /= dist;
		}

		Vector3 currentDir = velocity_;
		float speed = std::sqrt(currentDir.x * currentDir.x + currentDir.y * currentDir.y + currentDir.z * currentDir.z);
		if (speed > 0.0f) {
			currentDir.x /= speed;
			currentDir.y /= speed;
			currentDir.z /= speed;
		} else {
			currentDir = {0, 0, 1};
		}

		float dot = currentDir.x * toTarget.x + currentDir.y * toTarget.y + currentDir.z * toTarget.z;
		if (dot < 0.0f) {
			target_ = nullptr;
		} else {
			const float kHomingStrength = 0.1f;
			Vector3 newDir;
			newDir.x = LerpShort(currentDir.x, toTarget.x, kHomingStrength);
			newDir.y = LerpShort(currentDir.y, toTarget.y, kHomingStrength);
			newDir.z = LerpShort(currentDir.z, toTarget.z, kHomingStrength);

			float len = std::sqrt(newDir.x * newDir.x + newDir.y * newDir.y + newDir.z * newDir.z);
			if (len > 0.0f) {
				newDir.x /= len;
				newDir.y /= len;
				newDir.z /= len;
			}

			const float kMissileSpeed = 2.0f;
			velocity_ = {newDir.x * kMissileSpeed, newDir.y * kMissileSpeed, newDir.z * kMissileSpeed};

			worldTransform_.rotation_.y = std::atan2(velocity_.x, velocity_.z);
			float hLen = std::sqrt(velocity_.x * velocity_.x + velocity_.z * velocity_.z);
			worldTransform_.rotation_.x = -std::atan2(velocity_.y, hLen);
		}
	}

	worldTransform_.translation_ += velocity_;

	UpdateWorldMatrix(worldTransform_);
}

void PlayerMissile::Draw(const Camera& camera) { model_->Draw(worldTransform_, camera); }

void PlayerMissile::OnCollision() { isDead_ = true; }