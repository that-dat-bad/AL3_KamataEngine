#include "KamataEngine.h"
#include <Windows.h>
#include "GameScene.h"
#include "ResultScene.h"
#include "SceneManager.h"

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// エンジンの初期化
	KamataEngine::Initialize(L"跳弾");
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 各シーンのインスタンスを生成
	TitleScene* titleScene = new TitleScene();
	GameScene* gameScene = new GameScene();
	ResultScene* resultScene = new ResultScene();
	// 初期シーンを設定
	SceneManager currentScene = SceneManager::kTitle;

	// 初期化
	titleScene->Initialize();

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		KamataEngine::ImGuiManager::GetInstance()->Begin();

		SceneManager nextScene = currentScene;
		switch (currentScene) {
		case SceneManager::kTitle:
			nextScene = titleScene->Update();
			break;
		case SceneManager::kGame:
			nextScene = gameScene->Update();
			break;
		case SceneManager::kResult:
			 nextScene = resultScene->Update();
			break;
		}

		if (nextScene != currentScene) {
			if (nextScene == SceneManager::kGame) {
				gameScene->Initialize(GameScene::selectedStageIndex_);
			} 
			currentScene = nextScene;
		}

		dxCommon->PreDraw();

		switch (currentScene) {
		case SceneManager::kTitle:
			titleScene->Draw();
			break;
		case SceneManager::kGame:
			gameScene->Draw();
			break;
		case SceneManager::kResult:
			resultScene->Draw();
			break;
		}

		KamataEngine::ImGuiManager::GetInstance()->End();
		dxCommon->PostDraw();
	}

	// 解放処理
	delete titleScene;
	delete gameScene;

	KamataEngine::Finalize();
	return 0;
}