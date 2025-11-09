#include "EnemyBullet.h"
#include <cassert> 
#include "mathStruct.h" 

using namespace KamataEngine;

void EnemyBullet::Initialize(Model* model, const Vector3& position, const Vector3& velocity) {
	// NULLポインタチェック
	assert(model);

	model_ = model;
	// テクスチャ読み込み
	textureHandle_ = TextureManager::Load("white1x1.png"); 

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	// 引数で受け取った初期座標をセット
	worldTransform_.translation_ = position;

	// 引数で受け取った速度をメンバ変数に代入
	velocity_ = velocity;
}

void EnemyBullet::Update() {
	// 座標を移動させる
	worldTransform_.translation_ += velocity_;

	// 時間経過でデス
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}

	// ある一定程度（Z=0）まで近づいたら消滅
	if (worldTransform_.translation_.z < 0.0f) {
		isDead_ = true;
	}

	// ワールドトランスフォームの更新
	UpdateWorldMatrix(worldTransform_);
}

void EnemyBullet::Draw(const Camera& camera) {
	// モデルの描画
	model_->Draw(worldTransform_, camera, textureHandle_);
}