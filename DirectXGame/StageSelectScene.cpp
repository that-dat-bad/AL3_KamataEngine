#include "StageSelectScene.h"
#include "GameScene.h" 

void StageSelectScene::Initialize() { selectedStage_ = 0; }

Scene StageSelectScene::Update() {

	if (Input::GetInstance()->TriggerKey(DIK_RIGHT)) {
		selectedStage_++;
		if (selectedStage_ > 2) { 
			selectedStage_ = 0;
		}
	}

	if (Input::GetInstance()->TriggerKey(DIK_LEFT)) {
		selectedStage_--;
		if (selectedStage_ < 0) {
			selectedStage_ = 2;
		}
	}

	// スペースキーで決定
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		GameScene::SetSelectedStageIndex(selectedStage_);
		return Scene::kGame;
	}

	return Scene::kStageSelect;
}

void StageSelectScene::Draw() {


}