#include "Enemy.h"
#include "Player.h"
#include "mathStruct.h"
#include <DirectXMath.h>
#include <cassert>
#include <cmath>

using namespace KamataEngine;

Enemy::~Enemy() {
	for (EnemyBullet* bullet : bullets_) {
		delete bullet;
	}
}

void Enemy::Initialize(Model* model, const KamataEngine::Vector3& position) {
	assert(model);
	model_ = model;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;



	stateFunction_ = &Enemy::UpdateApproach;

	Fire();
}

void Enemy::Update() {
	bullets_.remove_if([](EnemyBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});

	for (EnemyBullet* bullet : bullets_) {
		bullet->Update();
	}

	(this->*stateFunction_)();

	if (worldTransform_.translation_.z < -10.0f || worldTransform_.translation_.z > 60.0f) {
		isDead_ = true;
	}

	UpdateWorldMatrix(worldTransform_);
}

void Enemy::Draw(const Camera& camera) {
	model_->Draw(worldTransform_, camera);
	for (EnemyBullet* bullet : bullets_) {
		bullet->Draw(camera);
	}
}

void Enemy::UpdateApproach() {
	// ★修正: プレイヤー追尾をやめて、まっすぐ手前に進む
	const float kSpeed = 0.2f;
	worldTransform_.translation_.z -= kSpeed;

	// 規定の位置（例えばZ=0）まで来たら離脱フェーズへ
	// もしプレイヤーとの距離で判定したい場合はここを調整します
	if (worldTransform_.translation_.z < 0.0f) {
		stateFunction_ = &Enemy::UpdateLeave;
	}
}

void Enemy::UpdateLeave() {
	// 離脱フェーズ：回転しながら飛び去る（ここは以前のまま）
	worldTransform_.rotation_.y += 0.02f;
	worldTransform_.rotation_.x += 0.01f;
	worldTransform_.rotation_.z += 0.02f;

	// 向いている方向に進む
	Vector3 move = {0, 0, -0.4f};
	Matrix4x4 matRot = MakeAffineMatrix({1.0f, 1.0f, 1.0f}, worldTransform_.rotation_, {0.0f, 0.0f, 0.0f});
	move = TransformNormal(move, matRot);

	worldTransform_.translation_.x += move.x;
	worldTransform_.translation_.y += move.y;
	worldTransform_.translation_.z += move.z;
}

void Enemy::Fire() {
	Vector3 position = worldTransform_.translation_;

	// ★修正: 本体を180度回転させているので、弾はプラス方向に出せば手前に飛ぶ
	const float kBulletSpeed = -0.5f;
	Vector3 velocity(0, 0, kBulletSpeed);

	Matrix4x4 matRot = MakeAffineMatrix({1.0f, 1.0f, 1.0f}, worldTransform_.rotation_, {0.0f, 0.0f, 0.0f});
	velocity = TransformNormal(velocity, matRot);

	EnemyBullet* newBullet = new EnemyBullet();
	newBullet->Initialize(bulletModel_, position, velocity);
	bullets_.push_back(newBullet);
}

void Enemy::OnCollision() { isDead_ = true; }