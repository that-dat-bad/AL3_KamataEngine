#include "Enemy.h"
#include "mathStruct.h"
#include <DirectXMath.h>
#include <cassert>

using namespace KamataEngine;

Enemy::~Enemy() {
	// 弾リストの解放
	for (EnemyBullet* bullet : bullets_) {
		delete bullet;
	}
}

void Enemy::Initialize(Model* model, const KamataEngine::Vector3& position) {
	assert(model);
	model_ = model;

	textureHandle_ = TextureManager::Load("UVChecker.png");

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	// 初期ステート（フェーズ）をメンバ関数ポインタで設定
	stateFunction_ = &Enemy::UpdateApproach;


	Fire();
}

void Enemy::Update() {
	// 弾の更新・削除処理
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

	// 現在のステートが指す関数を呼び出す
	(this->*stateFunction_)();

	// 画面外に行ったらデスフラグ
	if (worldTransform_.translation_.z < -10.0f || worldTransform_.translation_.z > 60.0f) {
		isDead_ = true;
	}

	// ワールドトランスフォームの更新
	UpdateWorldMatrix(worldTransform_);
}

void Enemy::Draw(const Camera& camera) {
	// 敵本体の描画
	model_->Draw(worldTransform_, camera, textureHandle_);

	// 弾の描画
	for (EnemyBullet* bullet : bullets_) {
		bullet->Draw(camera);
	}
}

void Enemy::UpdateApproach() {
	// 接近フェーズの速度
	const Vector3 velocityApproach = {0, 0, -0.2f};

	// 移動
	worldTransform_.translation_ += velocityApproach;

	// 規定の位置(Z=0)に到達したら離脱
	if (worldTransform_.translation_.z < 0.0f) {
		// ステートを離脱フェーズに変更
		stateFunction_ = &Enemy::UpdateLeave;
	}
}

void Enemy::UpdateLeave() {
	// 離脱フェーズの速度
	const Vector3 velocityLeave = {0, 0, 0.1f};

	// 移動
	worldTransform_.translation_ += velocityLeave;
}

void Enemy::Fire() {
	// 敵の座標をコピー
	Vector3 position = worldTransform_.translation_;

	// 弾の速度 (仮にZ-方向、プレイヤーより少し遅く)
	const float kBulletSpeed = -0.5f;
	Vector3 velocity(0, 0, kBulletSpeed);

	// 速度ベクトルを敵の向きに合わせて回転させる
	velocity = TransformNormal(velocity, worldTransform_.matWorld_);

	// 弾を生成し、初期化
	EnemyBullet* newBullet = new EnemyBullet();
	newBullet->Initialize(model_, position, velocity);

	// 弾を登録する
	bullets_.push_back(newBullet);
}