#include "TitleScene.h"
#include "GameScene.h" 

void TitleScene::Initialize() {
;
}

Scene TitleScene::Update() {
	// スペースキーが押されたらステージセレクトへ
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		return Scene::kStageSelect;
	}
	return Scene::kTitle;
}

void TitleScene::Draw() {
	
}