// ResultScene.cpp
#include "ResultScene.h"
#include "KamataEngine.h" // ★追加 (Inputなどを使う場合)
#include <optional>       // ★追加

// ★以下をすべて追加
ResultScene::~ResultScene() {}

void ResultScene::Initialize() {
	// (初期化処理)
}

std::optional<SceneID> ResultScene::Update() {
	// (更新処理)
	// 例: Enterが押されたらタイトルに戻る
	// if (KamataEngine::Input::GetInstance()->TriggerKey(DIK_RETURN)) {
	// 	return SceneID::kTitle;
	// }
	return std::nullopt;
}

void ResultScene::Draw() {
	// (描画処理)
}