#include "Enemy.h"
#include "mathStruct.h"

void Enemy::Initialize(Model* model, Camera* camera, const Vector3& position) {
	// nullポインタチェック
	assert(model);
	// 引数として受け取ったデータをメンバ変数に記録する
	model_ = model;
	camera_ = camera;
	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = 3.0f * std::numbers::pi_v<float> / 2.0f;

	velocity_ = {kWalkSpeed, 0.0f, 0.0f};
	walkTimer_ = 0.0f;
}

void Enemy::Update() {
	walkTimer_ += 1.0f / 60.0f; // タイマーを更新

worldTransform_.rotation_.x = (std::sin(walkTimer_ * (std::numbers::pi_v<float> / kWalkMotionTime)) * (std::numbers::pi_v<float> / 4.0f)) / 2.0f + (std::numbers::pi_v<float> / 8.0f);
	// 移動
	worldTransform_.translation_ += velocity_;

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Enemy::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Model::PreDraw(dxCommon->GetCommandList());
	model_->Draw(worldTransform_, *camera_);
	Model::PostDraw();
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

void Enemy::OnCollision(const Player* player) { (void)player; }
