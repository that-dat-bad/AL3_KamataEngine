#include "GameScene.h"
#include "Enemy.h"
#include "KamataEngine.h"
#include "Player.h"
#include <assert.h>

using namespace KamataEngine;

GameScene::~GameScene() {
	delete model_;
	delete player_;
	delete debugCamera_;

	// ★追加
	// 敵リストの解放
	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
}

void GameScene::Initialize() {
	textureHandle_ = TextureManager::Load("UVChecker.png");
	model_ = Model::Create();
	worldTransform_.Initialize();
	camera_.Initialize();
	player_ = new Player();
	player_->Initialize(model_, textureHandle_, &camera_);

	// Inputインスタンスの取得
	input_ = Input::GetInstance();

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);

	// 軸方向表示の表示を有効にする
	AxisIndicator::GetInstance()->SetVisible(true);
	// 軸方向表示が参照するビュープロジェクションを指定する
	AxisIndicator::GetInstance()->SetTargetCamera(&camera_);

	// ★追加
	// 敵の生成（例としてZ=50の位置に一体）
	Enemy* newEnemy = new Enemy();
	newEnemy->Initialize(model_, {0, 0, 50.0f});
	enemies_.push_back(newEnemy);
}

void GameScene::Update() {
	player_->Update();

	// ★追加
	// 敵の死亡処理
	enemies_.remove_if([](Enemy* enemy) {
		if (enemy->IsDead()) {
			delete enemy;
			return true;
		}
		return false;
	});

	// 敵の更新
	for (Enemy* enemy : enemies_) {
		enemy->Update();
	}

#ifdef _DEBUG
	// デバッグカメラ切り替え
	if (input_->TriggerKey(DIK_0)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}

	// デバッグカメラが有効な時だけImGuiウィンドウを表示
	if (isDebugCameraActive_) {
		ImGui::Begin("Debug Camera");
		ImGui::Text("Debug Camera: ON");
		ImGui::End();
	}

#endif

	// カメラの処理
	if (isDebugCameraActive_) {
		// デバッグカメラの更新
		debugCamera_->Update();
		// デバッグカメラのビュー行列・プロジェクション行列をメインカメラに設定
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		// ビュープロジェクション行列の転送
		camera_.TransferMatrix();
	} else {
		// ビュープロジェクション行列の更新と転送
		camera_.UpdateMatrix();
		camera_.TransferMatrix();
	}
}

void GameScene::Draw() {

	KamataEngine::DirectXCommon* dxCommon = KamataEngine::DirectXCommon::GetInstance();

	// モデルの描画準備
	KamataEngine::Model::PreDraw(dxCommon->GetCommandList());

	// プレイヤーの描画 (PreDraw/PostDrawを削除したもの)
	player_->Draw();

	// 敵の描画
	for (Enemy* enemy : enemies_) {
		enemy->Draw(camera_);
	}

	// モデルの描画終了
	KamataEngine::Model::PostDraw();

	// 軸方向表示の描画
	AxisIndicator::GetInstance()->Draw();
}