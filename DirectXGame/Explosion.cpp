#include "Explosion.h"
#include <cassert>
#include "mathStruct.h"

using namespace KamataEngine;

void Explosion::Initialize(Model* model, const Vector3& position) {
	assert(model);
	model_ = model;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	// 爆発の寿命
	timer_ = kLifeTime;
}

void Explosion::Update() {
	timer_--;
	if (timer_ <= 0) {
		isDead_ = true;
	}

	float growth = 1.0f;
	worldTransform_.scale_.x += growth;
	worldTransform_.scale_.y += growth;
	worldTransform_.scale_.z += growth;

	// 行列更新
	UpdateWorldMatrix(worldTransform_);
}

void Explosion::Draw(const Camera& camera) {
	if (!isDead_) {
		model_->Draw(worldTransform_, camera);
	}
}