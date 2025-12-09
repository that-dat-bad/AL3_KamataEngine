#include "SceneManager.h"
#include "GameScene.h"
#include "TitleScene.h"
#include "ResultScene.h"
#include <optional>

SceneManager::SceneManager() {}

SceneManager::~SceneManager() {
	if (currentScene_) {
		delete currentScene_;
		currentScene_ = nullptr;
	}
}

void SceneManager::Initialize() {
	currentScene_ = new TitleScene();
	currentScene_->Initialize();
	currentSceneID_ = SceneID::kTitle;

	#ifdef _DEBUG
	currentScene_ = new GameScene();
	currentScene_->Initialize();
	currentSceneID_ = SceneID::kGame;
#endif // _DEBUG
}

// 毎フレーム呼ばれる更新処理
void SceneManager::Update() {

	if (!currentScene_) {
		return;
	}

	// 4. 現在のシーンのUpdateを呼ぶ
	std::optional<SceneID> nextSceneID = currentScene_->Update();

	if (nextSceneID) {

		if (currentScene_) {
			delete currentScene_;
			currentScene_ = nullptr;
		}

		// 2. 新しいシーンを生成する
		currentSceneID_ = *nextSceneID;
		switch (currentSceneID_) {
		case SceneID::kTitle:
			 currentScene_ = new TitleScene();
			break;
		case SceneID::kGame:
			currentScene_ = new GameScene();
			break;
		case SceneID::kResult:
			currentScene_ = new ResultScene();
			break;
		}

		// 3. 新しいシーンを初期化する
		currentScene_->Initialize();
	}
}

// 毎フレーム呼ばれる描画処理
void SceneManager::Draw() {
	if (currentScene_) {
		currentScene_->Draw();
	}
}