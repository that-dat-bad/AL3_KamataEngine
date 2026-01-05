#include "Player.h"
#include "Enemy.h"
#include "KamataEngine.h"
#include "mathStruct.h"
#include <DirectXMath.h>
#include <algorithm>
#include <cassert>
#define _USE_MATH_DEFINES
#include <math.h>

using namespace KamataEngine;

Player::~Player() {
	for (PlayerBullet* bullet : bullets_) {
		delete bullet;
	}
	for (PlayerMissile* missile : missiles_) {
		delete missile;
	}
}

void Player::Initialize(KamataEngine::Model* model, Camera* camera) {
	assert(model);
	model_ = model;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.rotation_.y = static_cast<float>(M_PI); // 180度回転
	input_ = Input::GetInstance();

	hp_ = kMaxHP_;
	lives_ = kDefaultLives_;
	isDead_ = false;
	invincibleTimer_ = 0;
}

void Player::Update(bool isInputEnable) {
	// 無敵タイマー
	if (invincibleTimer_ > 0) {
		invincibleTimer_--;
	}

	// 弾・ミサイル削除
	bullets_.remove_if([](PlayerBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});
	missiles_.remove_if([](PlayerMissile* missile) {
		if (missile->IsDead()) {
			delete missile;
			return true;
		}
		return false;
	});

	// 更新処理
	for (PlayerBullet* bullet : bullets_) {
		bullet->Update();
	}
	for (PlayerMissile* missile : missiles_) {
		missile->Update();
	}

	// --- 自機の移動処理 ---
	Vector3 move = {0, 0, 0};
	const float kCharacterSpeed = 0.2f;

	float targetRotZ = 0.0f;
	float targetRotX = 0.0f;
	float targetRotY = static_cast<float>(M_PI);
	const float kMaxTilt = 0.5f;
	const float kMaxYaw = 0.3f;

	// 操作許可時のみ入力を受け付ける
	if (isInputEnable) {
		// ★変更: 矢印キーから WASD に変更
		// Aキー (左)
		if (input_->PushKey(DIK_A)) {
			move.x -= kCharacterSpeed;
			targetRotZ = kMaxTilt;
			targetRotY = static_cast<float>(M_PI) - kMaxYaw;
		}
		// Dキー (右)
		else if (input_->PushKey(DIK_D)) {
			move.x += kCharacterSpeed;
			targetRotZ = -kMaxTilt;
			targetRotY = static_cast<float>(M_PI) + kMaxYaw;
		}

		// Wキー (上)
		if (input_->PushKey(DIK_W)) {
			move.y += kCharacterSpeed;
			targetRotX = kMaxTilt;
		}
		// Sキー (下)
		else if (input_->PushKey(DIK_S)) {
			move.y -= kCharacterSpeed;
			targetRotX = -kMaxTilt;
		}

		// 攻撃
		Attack();
	}

	worldTransform_.translation_ += move;

	// 移動制限
	const float kMoveLimitX = 10.0f;
	const float kMoveLimitY = 5.0f;
	worldTransform_.translation_.x = (std::max)(worldTransform_.translation_.x, -kMoveLimitX);
	worldTransform_.translation_.x = (std::min)(worldTransform_.translation_.x, +kMoveLimitX);
	worldTransform_.translation_.y = (std::max)(worldTransform_.translation_.y, -kMoveLimitY);
	worldTransform_.translation_.y = (std::min)(worldTransform_.translation_.y, +kMoveLimitY);

	// 姿勢制御
	const float kTiltSpeed = 0.1f;
	worldTransform_.rotation_.z = LerpShort(worldTransform_.rotation_.z, targetRotZ, kTiltSpeed);
	worldTransform_.rotation_.x = LerpShort(worldTransform_.rotation_.x, targetRotX, kTiltSpeed);
	worldTransform_.rotation_.y = LerpShort(worldTransform_.rotation_.y, targetRotY, kTiltSpeed);

	UpdateWorldMatrix(worldTransform_);
}

void Player::Draw() {
	if (invincibleTimer_ % 4 < 2) {
		model_->Draw(worldTransform_, *camera_);
	}

	for (PlayerBullet* bullet : bullets_) {
		bullet->Draw(*camera_);
	}
	for (PlayerMissile* missile : missiles_) {
		missile->Draw(*camera_);
	}
}

void Player::Attack() {
	// ★変更: スペースキー(DIK_SPACE) から マウス左クリック(0) に変更
	// PushMouse(0)にすると押しっぱなしで連射になります。TriggerMouse(0)ならカチカチ連打。
	// ここでは押しっぱなし連射(Push)にします。
	if (input_->IsPressMouse(0)) {

		// 連射速度制限（これがないと毎フレーム発射されてすごいことになります）
		// 簡易的に10フレームに1回発射する例
		static int frameCount = 0;
		frameCount++;
		if (frameCount % 10 == 0) {
			Vector3 position = worldTransform_.translation_;
			const float kBulletSpeed = 1.0f;
			Vector3 velocity(0, 0, -kBulletSpeed);
			velocity = TransformNormal(velocity, worldTransform_.matWorld_);

			PlayerBullet* newBullet = new PlayerBullet();
			newBullet->Initialize(bulletModel_, position, velocity);
			bullets_.push_back(newBullet);
		}
	}
}

void Player::FireMissile(Enemy* target) {
	if (!target || target->IsDead())
		return;
	Vector3 position = worldTransform_.translation_;
	position.y -= 1.0f;
	PlayerMissile* newMissile = new PlayerMissile();
	newMissile->Initialize(missileModel_, position, target);
	missiles_.push_back(newMissile);
}

void Player::OnCollision() {
	if (invincibleTimer_ > 0)
		return;

	hp_--;
	invincibleTimer_ = 60;

	if (hp_ <= 0) {
		lives_--;
		if (lives_ > 0) {
			hp_ = kMaxHP_;
			invincibleTimer_ = 120;
		} else {
			isDead_ = true;
		}
	}
}