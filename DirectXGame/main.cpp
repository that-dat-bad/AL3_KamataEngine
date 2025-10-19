#include "KamataEngine.h"
#include <Windows.h>
#include "GameScene.h"
#include "StageSelectScene.h"
#include "TitleScene.h"

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// エンジンの初期化
	KamataEngine::Initialize(L"跳弾");
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 各シーンのインスタンスを生成
	TitleScene* titleScene = new TitleScene();
	StageSelectScene* stageSelectScene = new StageSelectScene();
	GameScene* gameScene = new GameScene();

	// 初期シーンを設定
	Scene currentScene = Scene::kTitle;

	// 初期化
	titleScene->Initialize();
	stageSelectScene->Initialize();

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		KamataEngine::ImGuiManager::GetInstance()->Begin();

		Scene nextScene = currentScene;
		switch (currentScene) {
		case Scene::kTitle:
			nextScene = titleScene->Update();
			break;
		case Scene::kStageSelect:
			nextScene = stageSelectScene->Update();
			break;
		case Scene::kGame:
			nextScene = gameScene->Update();
			break;
		}

		if (nextScene != currentScene) {
			if (nextScene == Scene::kGame) {
				gameScene->Initialize(GameScene::selectedStageIndex_);
			} else if (nextScene == Scene::kStageSelect) {
				stageSelectScene->Initialize();
			}
			currentScene = nextScene;
		}

		dxCommon->PreDraw();

		switch (currentScene) {
		case Scene::kTitle:
			titleScene->Draw();
			break;
		case Scene::kStageSelect:
			stageSelectScene->Draw();
			break;
		case Scene::kGame:
			gameScene->Draw();
			break;
		}

		KamataEngine::ImGuiManager::GetInstance()->End();
		dxCommon->PostDraw();
	}

	// 解放処理
	delete titleScene;
	delete stageSelectScene;
	delete gameScene;

	KamataEngine::Finalize();
	return 0;
}