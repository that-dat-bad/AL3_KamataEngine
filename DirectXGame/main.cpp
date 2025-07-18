#include "GameScene.h"
#include "KamataEngine.h"
#include "TitleScene.h"
#include <Windows.h>

using namespace KamataEngine;

// シーンの種類
enum class Scene {
	kUnknown,
	kTitle,
	kGame,
};

// 現在のシーン
Scene scene = Scene::kUnknown;
// 各シーンのインスタンスへのポインタ
TitleScene* titleScene = nullptr;
GameScene* gameScene = nullptr;

// --- シーン制御用の関数プロトタイプ宣言 ---
void ChangeScene(); // シーン切り替え
void UpdateScene(); // シーンごとの更新
void DrawScene();   // シーンごとの描画

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	// エンジンの初期化
	KamataEngine::Initialize(L"LE2B_14_タカナガ_ダイキ_AL3");
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// --- 最初のシーンをタイトルに設定 ---
	scene = Scene::kTitle;
	titleScene = new TitleScene();
	titleScene->Initialize();

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		// シーンの更新
		UpdateScene();
		// シーンの切り替え
		ChangeScene();

		// 描画開始
		dxCommon->PreDraw();
		// シーンの描画
		DrawScene();
		// 描画終了
		dxCommon->PostDraw();
	}

	// --- 終了処理 ---
	delete titleScene; // 最後のシーンがタイトルの場合を考慮
	delete gameScene;  // 最後のシーンがゲームの場合を考慮

	// エンジンの終了
	KamataEngine::Finalize();
	return 0;
}


void UpdateScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Update();
		break;
	case Scene::kGame:
		gameScene->Update();
		break;
	}
}

void DrawScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Draw();
		break;
	case Scene::kGame:
		gameScene->Draw();
		break;
	}
}

void ChangeScene() {
	switch (scene) {
	case Scene::kTitle:
		// タイトルシーンが終了したら
		if (titleScene->IsFinished()) {
			// GameSceneへ移行
			scene = Scene::kGame;
			// 古いシーンを解放
			delete titleScene;
			titleScene = nullptr;
			// 新しいシーンを作成・初期化
			gameScene = new GameScene();
			gameScene->Initialize();
		}
		break;
	case Scene::kGame:
		// ゲームシーンが終了したら
		if (gameScene->IsFinished()) {
			// TitleSceneへ移行
			scene = Scene::kTitle;
			// 古いシーンを解放
			delete gameScene;
			gameScene = nullptr;
			// 新しいシーンを作成・初期化
			titleScene = new TitleScene();
			titleScene->Initialize();
		}
		break;
	}
}