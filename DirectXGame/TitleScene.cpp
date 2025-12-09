#include "TitleScene.h"
#include "KamataEngine.h"
#include <assert.h>

using namespace KamataEngine;

TitleScene::~TitleScene() {
	delete model_;
	delete debugCamera_;
	delete fadeSprite_;
	delete titleLogoModel_;
	delete guideModel_;
}

void TitleScene::Initialize() {
	textureHandle_ = TextureManager::Load("UVChecker.png");
	model_ = Model::Create();
	worldTransform_.Initialize();
	camera_.Initialize();

	// Inputインスタンスの取得
	input_ = Input::GetInstance();

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);

	// --- フェーズとタイマーの初期化 ---
	phase_ = ScenePhase::kFadeIn;
	fadeTimer_ = kFadeDuration_;

	// --- フェード用スプライトの初期化 ---
	fadeTextureHandle_ = TextureManager::Load("white1x1.png");

	//---3Dモデルの初期化---
	//タイトルロゴ
	titleLogoModel_ = Model::CreateFromOBJ("title");
	titleLogoWorldTransform_.Initialize();
	titleLogoWorldTransform_.translation_ = {0.0f, 1.0f, 50.0f};
	// 誘導
	guideModel_ = Model::CreateFromOBJ("pressSpace");
	guideWorldTransform_.Initialize();
	guideWorldTransform_.translation_ = {0.0f, -2.0f, 5.0f};

	// パラメータをあらかじめ変数に用意
	Vector2 position = {0.0f, 0.0f};
	Vector2 size = {1280.0f, 720.0f};
	Vector4 color = {0.0f, 0.0f, 0.0f, 1.0f};
	Vector2 anchorpoint = {0.0f, 0.0f};

	// size や position を引数に渡すコンストラクタを使用する
	fadeSprite_ = new Sprite(
	    fadeTextureHandle_, position,
	    size, 
	    color, anchorpoint,
	    false, // isFlipX
	    false  // isFlipY
	);

	// ★コンストラクタで設定した後、Initialize() を呼び出す
	fadeSprite_->Initialize();

	// ★Initialize() の後で、テクスチャ範囲を 1x1 に設定
	fadeSprite_->SetTextureRect({0.0f, 0.0f}, {1.0f, 1.0f});
}

// Update() はフェーズの分岐管理のみ
std::optional<SceneID> TitleScene::Update() {
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

void TitleScene::Draw() {
	KamataEngine::DirectXCommon* dxCommon = KamataEngine::DirectXCommon::GetInstance();

	// --- 3Dモデル描画 ---
	KamataEngine::Model::PreDraw(dxCommon->GetCommandList());
	// (ここにタイトルロゴなどのモデル描画処理を追加できます)
	// model_->Draw(worldTransform_, camera_, textureHandle_);
	titleLogoModel_->Draw(titleLogoWorldTransform_, camera_);
	guideModel_->Draw(guideWorldTransform_, camera_);
	KamataEngine::Model::PostDraw();

	// --- フェードの描画処理 ---
	float alpha = 0.0f;
	if (phase_ == ScenePhase::kFadeIn) {
		// FadeIn 中は 1.0 -> 0.0 に変化 (黒いスプライトが消えていく)
		alpha = (float)fadeTimer_ / (float)kFadeDuration_;
	} else if (phase_ == ScenePhase::kFadeOut) {
		// FadeOut 中は 0.0 -> 1.0 に変化 (黒いスプライトが現れていく)
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

// FadeIn 中の処理
std::optional<SceneID> TitleScene::UpdateFadeIn() {
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
std::optional<SceneID> TitleScene::UpdateMain() {
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

	// ★スペースキーが押されたら FadeOut フェーズに移行
	if (input_->TriggerKey(DIK_SPACE)) {
		phase_ = ScenePhase::kFadeOut;
		fadeTimer_ = 0; // FadeOut用にタイマーリセット
	}

	// このフェーズ中はシーンを切り替えない
	return std::nullopt;
}

// FadeOut 中の処理
std::optional<SceneID> TitleScene::UpdateFadeOut() {
	// タイマーを増やす
	fadeTimer_++;

	// タイマーが指定時間に達したら
	if (fadeTimer_ >= kFadeDuration_) {
		// 次のシーン (kGame) を返す
		return SceneID::kGame;
	}

	// このフェーズ中はシーンを切り替えない
	return std::nullopt;
}