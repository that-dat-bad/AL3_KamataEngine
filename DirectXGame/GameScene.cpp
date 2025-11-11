#include "GameScene.h"
#include "Enemy.h"
#include "IScene.h"

#include "KamataEngine.h"
#include "Player.h"
#include <assert.h>

using namespace KamataEngine;

GameScene::~GameScene() {
	delete model_;
	delete player_;
	delete debugCamera_;
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

	// 敵の生成
	Enemy* newEnemy = new Enemy();
	newEnemy->Initialize(model_, {0, 0, 50.0f});
	enemies_.push_back(newEnemy);

	// フェーズとタイマーの初期化
	phase_ = ScenePhase::kFadeIn;
	fadeTimer_ = kFadeDuration_;
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

// Main 中の処理 (以前の Update() の中身)
std::optional<SceneID> GameScene::UpdateMain() {
	// --- (ここから) 以前の GameScene::Update() の中身 ---
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
	// --- (ここまで) 以前の GameScene::Update() の中身 ---

	// 仮: シーンを終了する条件 (例: Enterキーが押されたら)
	if (input_->TriggerKey(DIK_RETURN)) {
		// FadeOut フェーズに移行
		phase_ = ScenePhase::kFadeOut;
		fadeTimer_ = 0; // FadeOut用にタイマーリセット
	}

	// このフェーズ中はシーンを切り替えない
	return std::nullopt;
}

// FadeOut 中の処理
std::optional<SceneID> GameScene::UpdateFadeOut() {
	// タイマーを増やす
	fadeTimer_++;

	// タイマーが指定時間に達したら
	if (fadeTimer_ >= kFadeDuration_) {
		// 次のシーン (例: kResult) を返す
		return SceneID::kResult;
	}

	// このフェーズ中はシーンを切り替えない
	return std::nullopt;
}

void GameScene::Draw() {
	// 3Dオブジェクト描画処理
	KamataEngine::DirectXCommon* dxCommon = KamataEngine::DirectXCommon::GetInstance();
	KamataEngine::Model::PreDraw(dxCommon->GetCommandList());
	player_->Draw();
	for (Enemy* enemy : enemies_) {
		enemy->Draw(camera_);
	}
	KamataEngine::Model::PostDraw();
	AxisIndicator::GetInstance()->Draw();

	// --- フェードの描画処理 ---
	// (KamataEngineに2Dスプライト描画機能がある前提)

	float alpha = 0.0f;
	if (phase_ == ScenePhase::kFadeIn) {
		// FadeIn 中は 1.0 -> 0.0 に変化
		alpha = (float)fadeTimer_ / (float)kFadeDuration_;
	} else if (phase_ == ScenePhase::kFadeOut) {
		// FadeOut 中は 0.0 -> 1.0 に変化
		alpha = (float)fadeTimer_ / (float)kFadeDuration_;
	}

	// (アルファ付きで黒いスプライトを全画面に描画する処理をここに書く)
	// if (alpha > 0.0f) {
	// 	 sprite->Draw(blackTexture, {0,0}, {1280, 720}, {1,1,1, alpha});
	// }
}