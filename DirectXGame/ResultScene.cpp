#include "ResultScene.h"
#include "KamataEngine.h"
using namespace KamataEngine;

void ResultScene::Initialize() { ; }
SceneManager ResultScene::Update() {
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {

			return SceneManager::kTitle; 
	}
	return SceneManager::kResult;
}
void ResultScene::Draw() {


}
