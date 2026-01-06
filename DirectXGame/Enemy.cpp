#include "Enemy.h"
#include "Player.h"
#include "mathStruct.h"
#include <cassert>
#include <cmath>
#include <cstdlib> // rand()用

// 円周率
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace KamataEngine;

Enemy::~Enemy() {
	for (EnemyBullet* bullet : bullets_) {
		delete bullet;
	}
	for (EnemyMissile* missile : missiles_) {
		delete missile;
	}
}

void Enemy::Initialize(Model* model, const Vector3& position, const Vector3& velocity, const std::string& typeStr, const std::string& patternStr) {
	assert(model);
	model_ = model;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	velocity_ = velocity;

	// 1. タイプ設定 (HPと見た目)
	if (typeStr == "B") {
		type_ = EnemyType::TypeB;
		hp_ = 20;                                    // 硬い
		worldTransform_.scale_ = {2.0f, 2.0f, 2.0f}; // デカい
	} else {
		type_ = EnemyType::TypeA;
		hp_ = 3; // 普通
		worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	}

	// 2. 攻撃パターン設定
	if (patternStr == "Homing") {
		attackPattern_ = AttackPattern::Homing;
	} else if (patternStr == "None") {
		attackPattern_ = AttackPattern::None;
	} else {
		attackPattern_ = AttackPattern::Normal;
	}

	stateFunction_ = &Enemy::UpdateApproach;
	isDead_ = false;

	// ★追加: 初回の発射タイマーを設定 (60~200フレーム)
	shotTimer_ = rand() % 141 + 60;

	// 開幕攻撃（出現と同時に1発撃つ場合はこのまま残します）
	Fire();
}

void Enemy::Update() {
	// 弾更新
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

	// ミサイル更新
	missiles_.remove_if([](EnemyMissile* missile) {
		if (missile->IsDead()) {
			delete missile;
			return true;
		}
		return false;
	});
	for (EnemyMissile* missile : missiles_) {
		missile->Update();
	}

	// ★追加: 攻撃タイマー処理
	// 毎フレームカウントダウン
	shotTimer_--;
	if (shotTimer_ <= 0) {
		// 弾を発射
		Fire();
		// 次回の発射時間を再抽選 (60F ～ 200F)
		shotTimer_ = rand() % 141 + 60;
	}

	(this->*stateFunction_)();

	if (worldTransform_.translation_.z < -10.0f || worldTransform_.translation_.z > 1000.0f) {
		isDead_ = true;
	}
	UpdateWorldMatrix(worldTransform_);
}

void Enemy::Draw(const Camera& camera) {
	model_->Draw(worldTransform_, camera);
	for (EnemyBullet* bullet : bullets_) {
		bullet->Draw(camera);
	}
	for (EnemyMissile* missile : missiles_) {
		missile->Draw(camera);
	}
}

void Enemy::UpdateApproach() {
	worldTransform_.translation_.x += velocity_.x;
	worldTransform_.translation_.y += velocity_.y;
	worldTransform_.translation_.z += velocity_.z;

	// 姿勢制御
	float targetRotY = std::atan2(velocity_.x, velocity_.z) + static_cast<float>(M_PI);
	float targetRotZ = -velocity_.x * 2.0f;
	float targetRotX = -velocity_.y * 2.0f;

	const float kTiltSpeed = 0.1f;
	worldTransform_.rotation_.y = LerpShort(worldTransform_.rotation_.y, targetRotY, kTiltSpeed);
	worldTransform_.rotation_.z = LerpShort(worldTransform_.rotation_.z, targetRotZ, kTiltSpeed);
	worldTransform_.rotation_.x = LerpShort(worldTransform_.rotation_.x, targetRotX, kTiltSpeed);

	if (worldTransform_.translation_.z < 0.0f) {
		stateFunction_ = &Enemy::UpdateLeave;
	}
}

void Enemy::UpdateLeave() {
	worldTransform_.rotation_.y += 0.02f;
	worldTransform_.rotation_.x += 0.01f;
	worldTransform_.rotation_.z += 0.02f;
	Vector3 move = {0, 0, -0.4f};
	Matrix4x4 matRot = MakeAffineMatrix({1, 1, 1}, worldTransform_.rotation_, {0, 0, 0});
	move = TransformNormal(move, matRot);
	worldTransform_.translation_ += move;
}

void Enemy::Fire() {
	if (attackPattern_ == AttackPattern::None)
		return;

	Vector3 position = worldTransform_.translation_;

	if (attackPattern_ == AttackPattern::Normal) {
		// 通常弾
		Vector3 velocity = {0, 0, -0.5f};
		EnemyBullet* newBullet = new EnemyBullet();
		newBullet->Initialize(bulletModel_, position, velocity, false, nullptr);
		bullets_.push_back(newBullet);
	} else if (attackPattern_ == AttackPattern::Homing) {
		// ミサイル発射
		EnemyMissile* newMissile = new EnemyMissile();
		// プレイヤーをターゲットにする
		newMissile->Initialize(missileModel_, position, player_);
		missiles_.push_back(newMissile);
	}
}

void Enemy::OnCollision(int damage) {
	hp_ -= damage;
	if (hp_ <= 0) {
		isDead_ = true;
	}
}