#include "GameScene.h"
using namespace KamataEngine;



//デストラクタ
GameScene::~GameScene() {
	delete model_;
	delete player_;

}

void GameScene::Initialize() 
{
	// ファイル名を指定してテクスチャを読み込む
	textureHandle_ = TextureManager::Load("UVchecker.png");

	//3Dモデルの生成
	model_ = Model::Create();

	// カメラの初期化
	camera_.Initialize();

	//自キャラ生成
	player_ = new Player();
	// 自キャラの初期化
	player_->Initialize(model_, textureHandle_,&camera_);

}

void GameScene::Update() 
{
//自キャラの更新
	player_->Update();
}

void GameScene::Draw() 
{
	Model::PreDraw(dxCommon->GetCommandList());
	    //自キャラの描画
	player_->Draw();
	Model::PostDraw();
}
