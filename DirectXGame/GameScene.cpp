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

	// 3Dモデルの解放
	delete model_;

	delete debugCamera_;
}

/// <summary>
/// 初期化
/// </summary>
void GameScene::Initialize() {

	// ファイル名を指定してテクスチャを読み込む
	textureHandle_ = TextureManager::Load("uvChecker.png");

	// サウンドデータの読み込み
	soundDataHandle_ = Audio::GetInstance()->LoadWave("mokugyo.wav");

	// 音声データの再生
	Audio::GetInstance()->PlayWave(soundDataHandle_);

	// 音声データの再生
	voiceHandle_ = Audio::GetInstance()->PlayWave(soundDataHandle_,true);

	// スプライトインスタンスの生成
	sprite_ = Sprite::Create(textureHandle_, {100, 50});

	// 3Dモデルの生成
	model_ = Model::Create();

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();

	// カメラの初期化
	camera_.Initialize();

	//ライン描画が参照するカメラを指定する
	PrimitiveDrawer::GetInstance()->SetCamera(&camera_);
	debugCamera_ = new DebugCamera(1280,720);
	AxisIndicator::GetInstance()->SetVisible(true);
	AxisIndicator::GetInstance()->SetTargetCamera(&debugCamera_->GetCamera());
}

/// <summary>
/// 更新
/// </summary>
void GameScene::Update() {

	debugCamera_->Update();

	// スプライトの現在座標を取得(ゲッター)
	Vector2 pos = sprite_->GetPosition();

	//(1,2)移動
	pos.x += 1.0f;
	pos.y += 2.0f;

	// 移動した座標を反映(セッター)
	sprite_->SetPosition(pos);

	if (Input::GetInstance()->TriggerKey(DIK_SPACE))
	{
		// スペースキーが押されたら音声データを停止
		Audio::GetInstance()->StopWave(voiceHandle_);
	}
#ifdef _DEBUG
	ImGui::Begin("Debug1");
	ImGui::InputFloat3("InputFloat3", inputFloat);
	ImGui::SliderFloat3("SliderFloat3", inputFloat, 0.0f, 1.0f);

	ImGui::Text("Kamata Tarou %d.%d.%d", 2050, 12, 31);



	ImGui::End();
	ImGui::ShowDemoWindow();
#endif
}

/// <summary>
/// 描画
/// </summary>
void GameScene::Draw() {

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	////スプライト描画前処理
	// Sprite::PreDraw(dxCommon->GetCommandList());

	//
	// sprite_->Draw();

	////スプライトの描画後処理
	// Sprite::PostDraw();

	// 3Dモデルの描画前処理
	Model::PreDraw(dxCommon->GetCommandList());

	model_->Draw(worldTransform_, debugCamera_->GetCamera(), textureHandle_);

		PrimitiveDrawer::GetInstance()->DrawLine3d({0, 0, 0}, {0, 10, 0}, {1.0f, 0.0f, 0.0f, 1.0f});


	// 3Dモデルの描画後処理
	Model::PostDraw();
}
