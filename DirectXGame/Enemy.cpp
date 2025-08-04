#include "Enemy.h"
#include "Player.h"
#include "mathStruct.h"
#include "GameScene.h"

void Enemy::Initialize(Model* model, Camera* camera, const Vector3& position) {
	assert(model);
	model_ = model;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = 3.0f * std::numbers::pi_v<float> / 2.0f;
	velocity_ = {kWalkSpeed, 0.0f, 0.0f};
	walkTimer_ = 0.0f;
}

void Enemy::Update() {
	// ビヘイビアの遷移処理
	if (behaviorRequest_ != behavior_) {
		behavior_ = behaviorRequest_;
	}

	// ビヘイビアごとの更新処理
	switch (behavior_) {
	case Behavior::kWalk:
		BehaviorWalkUpdate();
		break;
	case Behavior::kDeath:
		BehaviorDeathUpdate();
		break;
	}

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Enemy::Draw() {
	Model::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	model_->Draw(worldTransform_, *camera_);
	Model::PostDraw();
}

void Enemy::BehaviorWalkUpdate() {
	walkTimer_ += 1.0f / 60.0f;
	worldTransform_.rotation_.x = (std::sin(walkTimer_ * (std::numbers::pi_v<float> / kWalkMotionTime)) * (std::numbers::pi_v<float> / 4.0f)) / 2.0f + (std::numbers::pi_v<float> / 8.0f);
	worldTransform_.translation_ += velocity_;
}

void Enemy::BehaviorDeathUpdate() {
	walkTimer_++;

	float t_y = static_cast<float>(walkTimer_) / kDeathDuration;
	worldTransform_.rotation_.y = EaseOut(0.0f, std::numbers::pi_v<float> * 4.0f, t_y);

	float t_x = static_cast<float>(walkTimer_) / kDeathDuration;
	worldTransform_.rotation_.x = EaseOut(0.0f, std::numbers::pi_v<float> / 2.0f, t_x);

	if (walkTimer_ >= kDeathDuration) {
		isDead_ = true;
	}
}

AABB Enemy::GetAABB() {
	Vector3 center = GetWorldPosition();
	AABB aabb;
	aabb.min.x = center.x - kWidth / 2.0f;
	aabb.max.x = center.x + kWidth / 2.0f;
	aabb.min.y = center.y - kHeight / 2.0f;
	aabb.max.y = center.y + kHeight / 2.0f;
	aabb.min.z = center.z - kWidth / 2.0f;
	aabb.max.z = center.z + kWidth / 2.0f;
	return aabb;
}

Vector3 Enemy::GetWorldPosition() {
	Vector3 worldPos;
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];
	return worldPos;
}

void Enemy::OnCollision(const Player* player) {
	if (behavior_ == Behavior::kDeath) {
		return;
	}
	if (player->IsAttack()) {
		behaviorRequest_ = Behavior::kDeath;
		walkTimer_ = 0;
		isCollisionDisabled_ = true;

		// 敵と自キャラの中間地点にエフェクトを生成
		Vector3 playerPos = player->GetWorldPosition();
		const Vector3& playerRot = player->GetWorldTransform().rotation_; // プレイヤーの回転を取得
		Vector3 enemyPos = GetWorldPosition();
		Vector3 effectPos = (playerPos + enemyPos) * 0.5f;
		gameScene_->CreateHitEffect(effectPos, playerRot);
	}
}