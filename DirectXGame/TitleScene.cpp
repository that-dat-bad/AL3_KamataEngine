#include "TitleScene.h"

TitleScene::~TitleScene() {
	delete fade_;
	delete titleImage_;
	delete backgroundSprite_;
}

void TitleScene::Initialize() {
	finished_ = false;
	selectedStage_ = 0;


	uint32_t whiteTextureHandle = TextureManager::Load("white1x1.png");
	backgroundSprite_ = Sprite::Create(whiteTextureHandle, {0, 0});
	backgroundSprite_->SetSize({1280.0f, 720.0f});
	backgroundSprite_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});

	uint32_t textureHandle = TextureManager::Load("png/title.png");
	titleImage_ = Sprite::Create(textureHandle, {0, 0});
	titleImage_->SetPosition({(1280.0f - 540.0f) / 2.0f, (720.0f - 680.0f) / 2.0f});

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
		// SPACEキーが押されたらゲーム開始
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			selectedStage_ = 1; // ステージ1へ
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, kFadeDuration);
		}
		// ENTERキーが押されたらステージセレクトへ
		else if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
			selectedStage_ = -1; // ステージセレクトへ
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

void TitleScene::Draw() {
	Sprite::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	backgroundSprite_->Draw();
	titleImage_->Draw();
	Sprite::PostDraw();

	fade_->Draw();
}