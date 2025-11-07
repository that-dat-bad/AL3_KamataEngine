#include "PlayerBullet.h"
#include "mathStruct.h"     // (スライド 154350.png / 154345.png)
#include <cassert>          // (スライド 154339.png)

using namespace KamataEngine;

// (スライド 154339.png)
void PlayerBullet::Initialize(Model* model, const Vector3& position) {
	// NULLポインタチェック
	assert(model);

	model_ = model;
	// テクスチャ読み込み
	textureHandle_ = TextureManager::Load("black.png");

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	// 引数で受け取った初期座標をセット
	worldTransform_.translation_ = position;
}

// (スライド 154350.png)
void PlayerBullet::Update() {
	// ワールドトランスフォームの更新
	UpdateWorldMatrix(worldTransform_);
}

// (スライド 154354.png)
void PlayerBullet::Draw(const Camera& camera) {
	// モデルの描画
	model_->Draw(worldTransform_, camera, textureHandle_);
}