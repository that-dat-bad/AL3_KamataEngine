#include "TitleScene.h"
#include "KamataEngine.h"
#include "SceneManager.h"
using namespace KamataEngine;
void TitleScene::Initialize() { ; }

SceneManager TitleScene::Update() {
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		return SceneManager::kGame;
	}
	return SceneManager::kTitle;
}

void TitleScene::Draw() {}