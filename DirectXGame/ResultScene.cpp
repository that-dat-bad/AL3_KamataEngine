#include "ResultScene.h"
#include "KamataEngine.h"
#include <cmath> // sin関数用

using namespace KamataEngine;

// static変数の実体定義
bool ResultScene::isWin = false;

ResultScene::~ResultScene() {
	// スプライトの解放
	delete bgSprite_;
	delete resultSprite_;
	delete rtbSprite_;
}

void ResultScene::Initialize() {
	// --- 背景の黒塗り ---
	// white1x1.png を読み込んで画面サイズに引き伸ばし、黒色にする
	bgTex_ = TextureManager::Load("white1x1.png");
	bgSprite_ = new Sprite(
	    bgTex_, {0.0f, 0.0f}, {1280.0f, 720.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, // 黒色 (R=0,G=0,B=0,A=1)
	    {0.0f, 0.0f}, false, false);
	bgSprite_->Initialize();

	// --- 勝敗画像の準備 ---
	winTex_ = TextureManager::Load("youWin.png");
	loseTex_ = TextureManager::Load("youLose.png");

	// isWinフラグを見て、使う画像を決める
	uint32_t resultTex = 0;
	if (isWin) {
		resultTex = winTex_;
	} else {
		resultTex = loseTex_;
	}

	// 中央に配置 (画面中央 x=640, y=360)
	// ★修正: サイズを 1280x200 に設定
	resultSprite_ = new Sprite(
	    resultTex, {640.0f, 360.0f}, {1280.0f, 200.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f}, // 中央揃え
	    false, false);
	resultSprite_->Initialize();

	// --- RTB (Return to Base) の準備 ---
	rtbTex_ = TextureManager::Load("RTB.png");

	// 画面下のほうに配置 (x=640, y=600)
	// ★修正: こちらも画像を 1280x200 と仮定して設定
	// もしRTBだけサイズが違う場合は、ここの数値を変更してください
	rtbSprite_ = new Sprite(
	    rtbTex_, {640.0f, 600.0f}, {1280.0f, 200.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f}, // 中央揃え
	    false, false);
	rtbSprite_->Initialize();

	// タイマー初期化
	blinkTimer_ = 0.0f;
}

std::optional<SceneID> ResultScene::Update() {
	// --- RTBの点滅処理 ---
	blinkTimer_ += 0.1f;
	// sin波でアルファ値を 0.0 ～ 1.0 に変動させる
	float alpha = (std::sin(blinkTimer_) + 1.0f) / 2.0f;

	if (rtbSprite_) {
		rtbSprite_->SetColor({1.0f, 1.0f, 1.0f, alpha});
	}

	// --- シーン遷移 ---
	// SPACEキーが押されたらタイトルへ
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		return SceneID::kTitle;
	}

	return std::nullopt;
}

void ResultScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// スプライト描画開始
	Sprite::PreDraw(dxCommon->GetCommandList(), Sprite::BlendMode::kNormal);

	// 1. 背景 (黒)
	if (bgSprite_)
		bgSprite_->Draw();

	// 2. 勝敗画像 (中央)
	if (resultSprite_)
		resultSprite_->Draw();

	// 3. RTB (下・点滅)
	if (rtbSprite_)
		rtbSprite_->Draw();

	// スプライト描画終了
	Sprite::PostDraw();
}