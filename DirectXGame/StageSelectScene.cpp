#include "StageSelectScene.h"
#include <cmath>
#include <string>

StageSelectScene::~StageSelectScene() {
	delete background_;
	for (int i = 0; i < kMaxStages; ++i) {
		delete sprites_[i];
	}
	delete fade_;
}

void StageSelectScene::Initialize() {
	uint32_t bgTexture = TextureManager::Load("png/select_background.png");
	background_ = Sprite::Create(bgTexture, {0.0f, 0.0f});
	background_->SetSize({1280.0f, 720.0f});

	for (int i = 0; i < kMaxStages; ++i) {
		uint32_t textureHandle = TextureManager::Load(("png/" + std::to_string(i + 1) + ".png").c_str());
		sprites_[i] = Sprite::Create(textureHandle, {0.0f, 0.0f});
		sprites_[i]->SetSize({150.0f, 150.0f});

		originalSpriteSizes_[i] = sprites_[i]->GetSize();
	}

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);
	phase_ = Phase::kFadeIn;
}

void StageSelectScene::Update() {
	fade_->Update();
	animationTimer_ += 1.0f / 60.0f;

	switch (phase_) {
	case Phase::kFadeIn:
		if (fade_->IsFinished()) {
			phase_ = Phase::kMain;
		}
		break;
	case Phase::kMain:
		if (Input::GetInstance()->TriggerKey(DIK_LEFT)) {
			selectedStage_--;
			if (selectedStage_ < 1) {
				selectedStage_ = kMaxStages;
			}
		}
		if (Input::GetInstance()->TriggerKey(DIK_RIGHT)) {
			selectedStage_++;
			if (selectedStage_ > kMaxStages) {
				selectedStage_ = 1;
			}
		}
		if (Input::GetInstance()->TriggerKey(DIK_SPACE) || Input::GetInstance()->TriggerKey(DIK_RETURN)) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, 1.0f);
		}
		break;
	case Phase::kFadeOut:
		if (fade_->IsFinished()) {
			isFinished_ = true;
		}
		break;
	}
}

void StageSelectScene::Draw() {
	Sprite::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	background_->Draw();

	for (int i = 0; i < kMaxStages; ++i) {

		Vector2 originalSize = originalSpriteSizes_[i];
		float scale = 1.0f;
		float yPos = 285.0f;

		if (i == selectedStage_ - 1) {
			scale = 1.1f + 0.1f * sinf(animationTimer_ * 5.0f);
		}

		Vector2 newSize = {originalSize.x * scale, originalSize.y * scale};
		sprites_[i]->SetSize(newSize);

		float xPos = (1280.0f / (kMaxStages + 1)) * (i + 1) - (newSize.x / 2.0f);

		sprites_[i]->SetPosition({xPos, yPos});

		sprites_[i]->Draw();
	}
	Sprite::PostDraw();

	fade_->Draw();
}