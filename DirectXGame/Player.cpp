#include "Player.h"

using namespace KamataEngine;

void Player::Initialize(KamataEngine::Model* model, uint32_t textureHandle) {
	assert(model);
	model_ = model;
	textureHandle_ = textureHandle;
	worldTransform_.Initialize();
}

void Player::Update() { 
	worldTransform_.TransferMatrix(); }

void Player::Draw() {}