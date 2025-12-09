#include "TitleScene.h"
#include "KamataEngine.h"
#include <assert.h>
#include"mathStruct.h"

using namespace KamataEngine;

TitleScene::~TitleScene() {
	delete debugCamera_;
	delete fadeSprite_;
	delete logo_;
	delete guide_;
	delete skydomeModel_;
}

void TitleScene::Initialize() {

	skydomeModel_ = Model::CreateFromOBJ("titleSkydome");

	skydomeTransform_.Initialize();
	skydomeTransform_.matWorld_ = MakeAffineMatrix(skydomeTransform_.scale_, skydomeTransform_.rotation_, skydomeTransform_.translation_);
	skydomeTransform_.TransferMatrix();
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
	logo_ = new TitleLogo();
	logo_->Initialize(Model::CreateFromOBJ("title"), &camera_, logoPosition_);
	logo_->SetPosition(logoPosition_);
	// 誘導
	guide_ = new TitleGuide();
	guide_->Initialize(Model::CreateFromOBJ("pressSpace"), &camera_, guidePosition_);
	guide_->SetPosition(guidePosition_);

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
	// 1. 結果を受け取る変数を用意 (初期値は nullopt)
	std::optional<SceneID> result = std::nullopt;

	// 2. フェーズごとの処理を実行し、結果を代入する (returnはしない！)
	switch (phase_) {
	case ScenePhase::kFadeIn:
		result = UpdateFadeIn();
		break;
	case ScenePhase::kMain:
		result = UpdateMain();
		break;
	case ScenePhase::kFadeOut:
		result = UpdateFadeOut();
		break;
	}

	// 3. 共通の更新処理 (ここが全フェーズで実行されるようになる！)

	// ImGuiなどで変えた位置を常に適用し続ける
	logo_->SetPosition(logoPosition_);
	logo_->Update();
	guide_->SetPosition(guidePosition_);
	guide_->Update();

	// カメラの更新
	// 一旦フェード中のズレを直すには最低限これを有効にする必要があります。
	if (!isDebugCameraActive_) {
		camera_.UpdateMatrix();
		camera_.TransferMatrix();
	}

	// 4. 最後に結果を返す
	return result;
}

void TitleScene::Draw() {
	KamataEngine::DirectXCommon* dxCommon = KamataEngine::DirectXCommon::GetInstance();

	// --- 3Dモデル描画 ---
	KamataEngine::Model::PreDraw(dxCommon->GetCommandList());
	// (ここにタイトルロゴなどのモデル描画処理を追加できます)
	// model_->Draw(worldTransform_, camera_, textureHandle_);
	logo_->Draw();
	guide_->Draw();
	skydomeModel_->Draw(skydomeTransform_, camera_);
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
	ImGui::Begin("Title Adjustment");
	ImGui::DragFloat3("Logo Position", &logoPosition_.x, 0.1f);
	ImGui::End();


#endif

	logo_->Update();
	guide_->Update();
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