#include "ClearScene.h"

ClearScene::~ClearScene() {
	delete fade_;
	delete clearTextSprite_;
}

void ClearScene::Initialize() {
	finished_ = false;

	// クリア画像のスプライトを生成
	uint32_t textureHandle = TextureManager::Load("png/game_clear.png");

	clearTextSprite_ = Sprite::Create(textureHandle, {0.0f, 0.0f});
	clearTextSprite_->SetSize({1280.0f, 720.0f});

	fade_ = new Fade();
	fade_->Initialize();

	// フェードインから開始
	phase_ = Phase::kFadeIn;
	fade_->Start(Fade::Status::FadeIn, 1.0f);
}

void ClearScene::Update() {
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
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, 1.0f);
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

void ClearScene::Draw() {
	// スプライト描画
	Sprite::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	if (clearTextSprite_) {
		clearTextSprite_->Draw();
	}
	Sprite::PostDraw();

	// フェード描画
	fade_->Draw();
}