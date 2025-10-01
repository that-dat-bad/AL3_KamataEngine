#include "GameScene.h"
#include "Player.h"
#include<assert.h>

using namespace KamataEngine;

GameScene::~GameScene() {
	delete model_;
	delete player_;
}

void GameScene::Initialize() {
	textureHandle_ = TextureManager::Load("uvChecker.png");
	model_ = Model::Create();
	worldTransform_.Initialize();
	camera_.Initialize();
	player_ = new Player();
}

void GameScene::Update() { player_->Update(); }

void GameScene::Draw() { player_->Draw(); }
