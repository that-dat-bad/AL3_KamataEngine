#include "Reticle.h"
#include "mathStruct.h"

using namespace KamataEngine;

void Reticle::Initialize() {

	textureHandle_ = TextureManager::Load("reticle.png");


	sprite_ = new Sprite(
	    textureHandle_, {0.0f, 0.0f}, {64.0f, 64.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.5f, 0.5f}, false, false 
	);
	sprite_->Initialize();
}

void Reticle::Update(const Vector3& targetWorldPos, const Camera& camera) {
	// 3D座標を2D画面座標に変換
	position_ = WorldToScreen(targetWorldPos, camera.matView, camera.matProjection, 1280.0f, 720.0f);

	// スプライトに座標を適用
	sprite_->SetPosition(position_);
}

void Reticle::Draw() {
	if (sprite_) {
		sprite_->Draw();
	}
}