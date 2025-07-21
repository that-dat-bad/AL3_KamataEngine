#include "TitleScene.h"

TitleScene::~TitleScene() { delete fade_; }

void TitleScene::Initialize() {
	finished_ = false;
	fade_ = new Fade();
	fade_->Initialize();

	// フェードインから開始
	phase_ = Phase::kFadeIn;
	fade_->Start(Fade::Status::FadeIn, kFadeDuration);
}

void TitleScene::Update() {
	// フェードの更新
	fade_->Update();

	// フェーズごとの処理
	switch (phase_) {
	case Phase::kFadeIn:
		// フェードインが終わったらメインフェーズへ
		if (fade_->IsFinished()) {
			phase_ = Phase::kMain;
		}
		break;
	case Phase::kMain:
		// スペースキーが押されたらフェードアウト開始
		if (Input::GetInstance()->PushKey(DIK_SPACE)) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, kFadeDuration);
		}
		break;
	case Phase::kFadeOut:
		// フェードアウトが終わったらシーン終了
		if (fade_->IsFinished()) {
			finished_ = true;
		}
		break;
	}
}

void TitleScene::Draw() { fade_->Draw(); }