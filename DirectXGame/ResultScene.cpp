#include "ResultScene.h"
#include "KamataEngine.h"
#include "mathStruct.h"
#include <string>

using namespace KamataEngine;

// 静的変数の実体定義
bool ResultScene::isWin = false;
int ResultScene::finalScore = 0;

ResultScene::~ResultScene() {
	delete skydomeModel_;
	delete fadeSprite_;
	delete winTextSprite_;
	delete loseResultSprite_;
	delete scoreBgSprite_;
	delete nextBtnSprite_;

	for (auto* sprite : scoreNumberSprites_) {
		delete sprite;
	}
	scoreNumberSprites_.clear();
}

void ResultScene::Initialize() {
	input_ = Input::GetInstance();

	// -------------------------------------------------
	// 1. 3D背景 (天球) の初期化
	// -------------------------------------------------
	camera_.Initialize();
	camera_.translation_.z = -10.0f; // カメラ位置調整

	skydomeModel_ = Model::CreateFromOBJ("Skydome");
	skydomeTransform_.Initialize();
	skydomeTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	skydomeTransform_.matWorld_ = MakeAffineMatrix(skydomeTransform_.scale_, skydomeTransform_.rotation_, skydomeTransform_.translation_);
	skydomeTransform_.TransferMatrix();

	// -------------------------------------------------
	// 2. テクスチャ読み込み
	// -------------------------------------------------
	clearTexHandle_ = TextureManager::Load("gameclear.png");
	whiteTexHandle_ = TextureManager::Load("white1x1.png");
	returnTexHandle_ = TextureManager::Load("return.png");

	// 数字画像 (0.png ~ 9.png) を読み込む
	for (int i = 0; i < 10; i++) {
		std::string filename = std::to_string(i) + ".png";
		numberTexHandles_[i] = TextureManager::Load(filename.c_str());
	}

	// -------------------------------------------------
	// 3. 結果表示スプライト (勝ち/負け)
	// -------------------------------------------------
	// 勝利時: CLEAR画像
	winTextSprite_ = new Sprite(clearTexHandle_, {640, 200}, {600, 150}, {1, 1, 1, 1}, {0.5f, 0.5f}, false, false);
	winTextSprite_->Initialize();

	// 敗北時: 赤い四角 (GAME OVERの代わり)
	loseResultSprite_ = new Sprite(whiteTexHandle_, {640, 200}, {600, 150}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.5f, 0.5f}, false, false);
	loseResultSprite_->Initialize();

	// -------------------------------------------------
	// 4. スコア表示 (白背景 + 数字)
	// -------------------------------------------------

	// ★修正: 数字サイズ(64x128)に合わせて計算
	float numberWidth = 64.0f;
	float numberHeight = 128.0f;

	std::string scoreStr = std::to_string(finalScore);
	int digits = (int)scoreStr.length();

	// 数字全体の幅 = 桁数 * 1文字幅
	float totalNumWidth = digits * numberWidth;

	// スコア背景 (白い帯)
	// 数字全体より少し大きくする (横幅+40, 縦幅+20 くらい)
	scoreBgSprite_ = new Sprite(whiteTexHandle_, {640, 450}, {totalNumWidth + 40.0f, numberHeight + 20.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f}, false, false);
	scoreBgSprite_->Initialize();

	// 数字スプライト生成
	// 開始X座標 (中央寄せ)
	float startX = 640.0f - (totalNumWidth / 2.0f) + (numberWidth / 2.0f);

	for (int i = 0; i < digits; ++i) {
		int digit = scoreStr[i] - '0';
		Sprite* numSprite = new Sprite(
		    numberTexHandles_[digit], {startX + i * numberWidth, 450.0f}, // Y座標は背景と同じ高さ
		    {numberWidth, numberHeight},                                  // ★サイズ指定 64x128
		    {1.0f, 1.0f, 1.0f, 1.0f},                                     // そのままの色で表示
		    {0.5f, 0.5f}, false, false);
		numSprite->Initialize();
		scoreNumberSprites_.push_back(numSprite);
	}

	// -------------------------------------------------
	// 5. ボタン誘導 (return.png)
	// -------------------------------------------------
	// サイズ 300x40
	nextBtnSprite_ = new Sprite(returnTexHandle_, {640, 600}, {300, 40}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f}, false, false);
	nextBtnSprite_->Initialize();

	// -------------------------------------------------
	// 6. フェード
	// -------------------------------------------------
	fadeTexHandle_ = TextureManager::Load("white1x1.png");
	fadeSprite_ = new Sprite(fadeTexHandle_, {0, 0}, {1280, 720}, {1, 1, 1, 1}, {0, 0}, false, false);
	fadeSprite_->Initialize();
	fadeSprite_->SetTextureRect({0, 0}, {1, 1});

	phase_ = ScenePhase::kFadeIn;
	fadeTimer_ = kFadeDuration;
}

std::optional<SceneID> ResultScene::Update() {
	// フェードイン
	if (phase_ == ScenePhase::kFadeIn) {
		fadeTimer_--;
		if (fadeTimer_ <= 0) {
			phase_ = ScenePhase::kMain;
		}
	}
	// フェードアウト
	else if (phase_ == ScenePhase::kFadeOut) {
		fadeTimer_++;
		if (fadeTimer_ >= kFadeDuration) {
			return SceneID::kTitle;
		}
	}
	// メインループ
	else if (phase_ == ScenePhase::kMain) {
		// スペースキーでタイトルへ
		if (input_->TriggerKey(DIK_SPACE)) {
			phase_ = ScenePhase::kFadeOut;
			fadeTimer_ = 0;
		}
	}

	return std::nullopt;
}

void ResultScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 1. 天球描画 (3D)
	Model::PreDraw(dxCommon->GetCommandList());
	if (skydomeModel_) {
		skydomeModel_->Draw(skydomeTransform_, camera_);
	}
	Model::PostDraw();

	// 2. UI描画 (2D)
	Sprite::PreDraw(dxCommon->GetCommandList(), Sprite::BlendMode::kNormal);

	// 結果表示 (勝ち/負け)
	if (isWin) {
		if (winTextSprite_)
			winTextSprite_->Draw();
	} else {
		if (loseResultSprite_)
			loseResultSprite_->Draw();
	}

	// スコア背景 (白い帯)
	if (scoreBgSprite_)
		scoreBgSprite_->Draw();

	// スコア数字 (背景の上に描画)
	for (auto* sprite : scoreNumberSprites_) {
		sprite->Draw();
	}

	// ボタン誘導 (return.png)
	if (nextBtnSprite_)
		nextBtnSprite_->Draw();

	// フェード
	float alpha = 0.0f;
	if (phase_ == ScenePhase::kFadeIn)
		alpha = (float)fadeTimer_ / kFadeDuration;
	else if (phase_ == ScenePhase::kFadeOut)
		alpha = (float)fadeTimer_ / kFadeDuration;

	if (alpha > 0.0f && fadeSprite_) {
		// 黒フェード
		fadeSprite_->SetColor({0, 0, 0, alpha});
		fadeSprite_->Draw();
	}

	Sprite::PostDraw();
}