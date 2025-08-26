#include "StageSelectScene.h"
#include <string>
#include "KamataEngineExtensions.h"

void StageSelectScene::Initialize() {
	for (int i = 0; i < kMaxStages; ++i) {
		uint32_t textureHandle = TextureManager::Load(("png/" + std::to_string(i + 1) + ".png").c_str());
		sprites_[i] = Sprite::Create(textureHandle, {0.0f, 0.0f});
		// 中央に配置
		sprites_[i]->SetPosition({(1280.0f / (kMaxStages + 1)) * (i + 1) - (sprites_[i]->GetSize().x / 2.0f), 300.0f});
	}
}

void StageSelectScene::Update() {
	// ←キーで選択を左へ
	if (Input::GetInstance()->TriggerKey(DIK_LEFT)) {
		selectedStage_--;
		if (selectedStage_ < 1) {
			selectedStage_ = kMaxStages;
		}
	}
	// →キーで選択を右へ
	if (Input::GetInstance()->TriggerKey(DIK_RIGHT)) {
		selectedStage_++;
		if (selectedStage_ > kMaxStages) {
			selectedStage_ = 1;
		}
	}

	// スペースキーで決定
	if (Input::GetInstance()->TriggerKey(DIK_SPACE) || Input::GetInstance()->TriggerKey(DIK_RETURN)) {
		isFinished_ = true;
	}

}

void StageSelectScene::Draw() {
	Sprite::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	// 全てのスプライトを描画
	for (int i = 0; i < kMaxStages; ++i) {
		// 選択中のものは少し大きくする
		if (i == selectedStage_ - 1) {
			KamataEngineExtensions::SetSpriteScale(sprites_[i], 1.2f);
		} else {
			KamataEngineExtensions::SetSpriteScale(sprites_[i], 1.0f);
		}
		sprites_[i]->Draw();
	}

	Sprite::PostDraw();
}