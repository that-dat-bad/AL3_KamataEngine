#include "GameScene.h"
#include "Player.h"
#include<assert.h>
using namespace KamataEngine;

using namespace KamataEngine;

GameScene::~GameScene() {
	delete model_;
	delete player_;
}

void GameScene::Initialize() {
	textureHandle_ = TextureManager::Load("UVChecker.png");
	model_ = Model::Create();
	worldTransform_.Initialize();
	camera_.Initialize();
	player_ = new Player();
	player_->Initialize(model_, textureHandle_, &camera_);
}

void GameScene::Update() { 
	player_->Update(); 
}

void GameScene::Draw() { player_->Draw(); }
