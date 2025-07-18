#include "TitleScene.h"

void TitleScene::Initialize() { finished_ = false; }

void TitleScene::Update() {
	// スペースキーが押されたら終了フラグを立てる
	if (Input::GetInstance()->PushKey(DIK_SPACE)) {
		finished_ = true;
	}
}

void TitleScene::Draw() {
	// 今は描画するものが何もない
}