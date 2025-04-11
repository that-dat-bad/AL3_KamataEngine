#include "GameScene.h"
#include "KamataEngine.h"
using namespace KamataEngine;


GameScene::GameScene() {}

/// <summary>
/// デストラクタ
/// </summary>
GameScene::~GameScene() { 
	// スプライトの解放
	delete sprite_; 
}


/// <summary>
/// 初期化
/// </summary>
void GameScene::Initialize() {

	//ファイル名を指定してテクスチャを読み込む
	textureHandle_ = TextureManager::Load("uvChecker.png");

	//スプライトインスタンスの生成
	sprite_ = Sprite::Create(textureHandle_, {100, 50});
}


/// <summary>
/// 更新
/// </summary>
void GameScene::Update() {
	
	//スプライトの現在座標を取得(ゲッター)
	Vector2 pos = sprite_->GetPosition();
	
	//(1,2)移動
	pos.x += 1.0f;
	pos.y += 2.0f;

	//移動した座標を反映(セッター)
	sprite_->SetPosition(pos);
}


/// <summary>
/// 描画
/// </summary>
void GameScene::Draw() {

	//DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	//スプライト描画前処理
	Sprite::PreDraw(dxCommon->GetCommandList());
	
	sprite_->Draw();

	//スプライトの描画後処理	
	Sprite::PostDraw();

}
