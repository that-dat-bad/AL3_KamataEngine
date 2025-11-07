#include "Player.h"
#include "mathStruct.h"
#include <algorithm>
#include <cassert>

using namespace KamataEngine;

void Player::Initialize(KamataEngine::Model* model, uint32_t textureHandle, Camera* camera) {
	assert(model);
	model_ = model;
	textureHandle_ = textureHandle;
	camera_ = camera;
	worldTransform_.Initialize();

	// シングルトンインスタンスを取得
	input_ = Input::GetInstance();
}

void Player::Update() {

	// キャラクターの移動ベクトル
	Vector3 move = {0, 0, 0};

	// キャラクターの移動速さ
	const float kCharacterSpeed = 0.2f;

	// 押した方向で移動ベクトルを変更（左右）
	if (input_->PushKey(DIK_LEFT)) {
		move.x -= kCharacterSpeed;
	} else if (input_->PushKey(DIK_RIGHT)) {
		move.x += kCharacterSpeed;
	}

	// 押した方向で移動ベクトルを変更（上下）
	if (input_->PushKey(DIK_UP)) {
		move.y += kCharacterSpeed;
	} else if (input_->PushKey(DIK_DOWN)) {
		move.y -= kCharacterSpeed;
	}

	// 座標移動（ベクトルの加算）
	worldTransform_.translation_ += move;

	// 旋回（回転）
	const float kRotSpeed = 0.02f;
	if (input_->PushKey(DIK_A)) {
		worldTransform_.rotation_.y -= kRotSpeed;
	} else if (input_->PushKey(DIK_D)) {
		worldTransform_.rotation_.y += kRotSpeed;
	}

	// 移動限界座標
	const float kMoveLimitX = 10.0f;
	const float kMoveLimitY = 5.0f;

	// 範囲を超えない処理
	worldTransform_.translation_.x = (std::max)(worldTransform_.translation_.x, -kMoveLimitX);
	worldTransform_.translation_.x = (std::min)(worldTransform_.translation_.x, +kMoveLimitX);
	worldTransform_.translation_.y = (std::max)(worldTransform_.translation_.y, -kMoveLimitY);
	worldTransform_.translation_.y = (std::min)(worldTransform_.translation_.y, +kMoveLimitY);

	// キャラクター攻撃処理
	Attack();

	// 弾更新
	if (bullet_) {
		bullet_->Update();
	}

	// キャラクターの座標を画面表示する処理
	ImGui::Begin("Player Info");
	ImGui::Text("Translation: (%.2f, %.2f, %.2f)", worldTransform_.translation_.x, worldTransform_.translation_.y, worldTransform_.translation_.z);
	ImGui::End();

	// ワールド行列更新
	UpdateWorldMatrix(worldTransform_);
}

void Player::Draw() {
	KamataEngine::DirectXCommon* dxCommon = KamataEngine::DirectXCommon::GetInstance();

	KamataEngine::Model::PreDraw(dxCommon->GetCommandList());

	model_->Draw(worldTransform_, *camera_, textureHandle_);

	if (bullet_) {
		bullet_->Draw(*camera_);
	}

	KamataEngine::Model::PostDraw();
}

void Player::Attack() {
	if (input_->TriggerKey(DIK_SPACE)) {

		// 弾を生成し、初期化
		PlayerBullet* newBullet = new PlayerBullet();
		newBullet->Initialize(model_, worldTransform_.translation_);

		// 弾を登録する
		bullet_ = newBullet;
	}
}