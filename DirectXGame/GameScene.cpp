#include "GameScene.h"
#include "Enemy.h"
#include "IScene.h"
#include "KamataEngine.h"
#include "Player.h"
#include <assert.h>

using namespace KamataEngine;

GameScene::~GameScene() {
	delete playerModel_;
	delete enemyModel_;
	delete player_;
	delete debugCamera_;
	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	delete fadeSprite_; // スプライトの解放
}

void GameScene::Initialize() {
	//textureHandle_ = TextureManager::Load("UVChecker.png");
	playerModel_ = Model::CreateFromOBJ("player");
	enemyModel_ = Model::CreateFromOBJ("enemy");


	worldTransform_.Initialize();
	camera_.Initialize();

	// --- カメラの位置を調整 ---
	camera_.translation_ = {0.0f, 2.5f, -15.0f};
	// ---

	player_ = new Player();
	// ★プレイヤーモデルを渡す
	player_->Initialize(playerModel_, textureHandle_, &camera_);

	// Inputインスタンスの取得
	input_ = Input::GetInstance();

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);


	// 敵の生成
	Enemy* newEnemy = new Enemy();
	// ★敵モデルを渡す
	newEnemy->Initialize(enemyModel_, {0, 0, 50.0f});
	enemies_.push_back(newEnemy);

	// フェーズとタイマーの初期化
	phase_ = ScenePhase::kFadeIn;
	fadeTimer_ = kFadeDuration_;

	// --- フェード用スプライトの初期化 ---
	fadeTextureHandle_ = TextureManager::Load("white1x1.png");
	Vector2 position = {0.0f, 0.0f};
	Vector2 size = {1280.0f, 720.0f};
	Vector4 color = {0.0f, 0.0f, 0.0f, 1.0f};
	Vector2 anchorpoint = {0.0f, 0.0f};
	fadeSprite_ = new Sprite(fadeTextureHandle_, position, size, color, anchorpoint, false, false);
	fadeSprite_->Initialize();
	fadeSprite_->SetTextureRect({0.0f, 0.0f}, {1.0f, 1.0f});
}

// Update() はフェーズの分岐管理のみ
std::optional<SceneID> GameScene::Update() {
	switch (phase_) {
	case ScenePhase::kFadeIn:
		return UpdateFadeIn();
	case ScenePhase::kMain:
		return UpdateMain();
	case ScenePhase::kFadeOut:
		return UpdateFadeOut();
	}
	return std::nullopt;
}

// FadeIn 中の処理
std::optional<SceneID> GameScene::UpdateFadeIn() {
	// タイマーを減らす
	fadeTimer_--;

	// タイマーが0になったら Main フェーズに移行
	if (fadeTimer_ <= 0) {
		phase_ = ScenePhase::kMain;
	}

	// このフェーズ中はシーンを切り替えない
	return std::nullopt;
}

// Main 中の処理
std::optional<SceneID> GameScene::UpdateMain() {
	player_->Update();

	enemies_.remove_if([](Enemy* enemy) {
		if (enemy->IsDead()) {
			delete enemy;
			return true;
		}
		return false;
	});

	for (Enemy* enemy : enemies_) {
		enemy->Update();
	}

#ifdef _DEBUG
	if (input_->TriggerKey(DIK_0)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}
	if (isDebugCameraActive_) {
		ImGui::Begin("Debug Camera");
		ImGui::Text("Debug Camera: ON");
		ImGui::End();
	}
#endif

	if (isDebugCameraActive_) {
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		camera_.TransferMatrix();
	} else {
		camera_.UpdateMatrix();
		camera_.TransferMatrix();
	}

	if (input_->TriggerKey(DIK_RETURN)) {
		phase_ = ScenePhase::kFadeOut;
		fadeTimer_ = 0;
	}
	return std::nullopt;
}

// FadeOut 中の処理
std::optional<SceneID> GameScene::UpdateFadeOut() {
	// タイマーを増やす
	fadeTimer_++;

	// タイマーが指定時間に達したら
	if (fadeTimer_ >= kFadeDuration_) {
		return SceneID::kResult;
	}
	return std::nullopt;
}

void GameScene::Draw() {
	KamataEngine::DirectXCommon* dxCommon = KamataEngine::DirectXCommon::GetInstance();

	// 3Dオブジェクト描画処理
	// フェードイン中は3Dモデルを描画しない
	if (phase_ != ScenePhase::kFadeIn) {
		KamataEngine::Model::PreDraw(dxCommon->GetCommandList());
		player_->Draw();
		for (Enemy* enemy : enemies_) {
			enemy->Draw(camera_);
		}
		KamataEngine::Model::PostDraw();
	}

	// --- フェードの描画処理 ---
	float alpha = 0.0f;
	if (phase_ == ScenePhase::kFadeIn) {
		// FadeIn 中は 1.0 -> 0.0 に変化
		alpha = (float)fadeTimer_ / (float)kFadeDuration_;
	} else if (phase_ == ScenePhase::kFadeOut) {
		// FadeOut 中は 0.0 -> 1.0 に変化
		alpha = (float)fadeTimer_ / (float)kFadeDuration_;
	}

	// アルファ値が0より大きい場合のみスプライトを描画
	if (alpha > 0.0f && fadeSprite_) {
		// スプライトの描画前処理 (通常ブレンド)
		Sprite::PreDraw(dxCommon->GetCommandList(), Sprite::BlendMode::kNormal);

		// スプライトの色を設定 (R,G,B = 0 (黒), A = alpha)
		fadeSprite_->SetColor({0.0f, 0.0f, 0.0f, alpha});

		// スプライト描画
		fadeSprite_->Draw();

		// スプライトの描画後処理
		Sprite::PostDraw();
	}
}