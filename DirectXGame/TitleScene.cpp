#include "TitleScene.h"
#include "KamataEngine.h"
#include "SceneManager.h"
using namespace KamataEngine;
void TitleScene::Initialize() {
	titleTexture_ = TextureManager::Load("/Title/title.png");
	textTexture_ = TextureManager::Load("/Title/press_space.png");
}

SceneManager TitleScene::Update() {
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		return SceneManager::kGame;
	}
	return SceneManager::kTitle;
}

void TitleScene::Draw() {
	Sprite::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	Sprite* titleSprite = Sprite::Create(titleTexture_, {WinApp::kWindowWidth / 2.0f, WinApp::kWindowHeight / 2.0f}, {1, 1, 1, 1}, {0.5f, 0.5f});
	titleSprite->Draw();
	Sprite* textSprite = Sprite::Create(textTexture_, {WinApp::kWindowWidth / 2.0f, WinApp::kWindowHeight - 100.0f}, {1, 1, 1, 1}, {0.5f, 0.5f});
	textSprite->Draw();
	Sprite::PostDraw();
}