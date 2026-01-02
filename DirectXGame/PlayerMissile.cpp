#include "PlayerMissile.h"
#include "Enemy.h" // Enemyの座標を知るため
#include "mathStruct.h"
#include <cassert>
#include <cmath>

using namespace KamataEngine;

void PlayerMissile::Initialize(Model* model, const Vector3& position, Enemy* target) {
	assert(model);
	model_ = model;
	// もしミサイル用のテクスチャがあればここを変更
	textureHandle_ = TextureManager::Load("white1x1.png");

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	target_ = target;

	// 初期速度：最初は上方向に打ち上げてから誘導開始する（かっこいい挙動用）
	// 少しランダムに散らすとさらにミサイルっぽい
	velocity_ = {0.0f, 0.5f, 0.0f};
}

void PlayerMissile::Update() {
	// ターゲットが生きていれば誘導する
	if (target_ && !target_->IsDead()) {
		Vector3 targetPos = target_->GetWorldPosition();
		Vector3 myPos = worldTransform_.translation_;

		Vector3 toTarget;
		toTarget.x = targetPos.x - myPos.x;
		toTarget.y = targetPos.y - myPos.y;
		toTarget.z = targetPos.z - myPos.z;

		// 正規化
		float len = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y + toTarget.z * toTarget.z);
		if (len > 0.0f) {
			toTarget.x /= len;
			toTarget.y /= len;
			toTarget.z /= len;
		}

		// 現在の進行方向とターゲット方向を混ぜる（誘導）
		velocity_.x = velocity_.x * (1.0f - kHomingStrength_) + toTarget.x * kHomingStrength_;
		velocity_.y = velocity_.y * (1.0f - kHomingStrength_) + toTarget.y * kHomingStrength_;
		velocity_.z = velocity_.z * (1.0f - kHomingStrength_) + toTarget.z * kHomingStrength_;
	}

	// 速度を一定に保つ
	float vLen = std::sqrt(velocity_.x * velocity_.x + velocity_.y * velocity_.y + velocity_.z * velocity_.z);
	if (vLen > 0.0f) {
		velocity_.x = (velocity_.x / vLen) * kMissileSpeed_;
		velocity_.y = (velocity_.y / vLen) * kMissileSpeed_;
		velocity_.z = (velocity_.z / vLen) * kMissileSpeed_;
	}

	// 移動
	worldTransform_.translation_ += velocity_;

	// 向きを進行方向に合わせる（簡易版）
	// 本来はatan2などで回転行列を作るが、今回はモデルが球体などであれば不要
	// ミサイル形状の場合は回転計算が必要ですが、一旦位置更新のみとします

	// 寿命処理
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}

	// 行列更新
	UpdateWorldMatrix(worldTransform_);
}

void PlayerMissile::Draw(const Camera& camera) {
	// ミサイルは赤色などで描画
	model_->Draw(worldTransform_, camera, textureHandle_);
}

void PlayerMissile::OnCollision() { isDead_ = true; }