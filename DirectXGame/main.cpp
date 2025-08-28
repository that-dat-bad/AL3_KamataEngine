#include "ClearScene.h"
#include "GameScene.h"
#include "KamataEngine.h"
#include "StageSelectScene.h"
#include "TitleScene.h"
#include <Windows.h>

using namespace KamataEngine;

// シーンの種類
enum class Scene {
	kUnknown,
	kTitle,
	kStageSelect,
	kGame,
	kClear,
};

// 現在のシーン
Scene scene = Scene::kUnknown;
// ステージの総数
const int kMaxStages = 2;
// 現在のステージ番号を管理する変数
static int g_currentStage = 1;

// 各シーンのインスタンスへのポインタ
TitleScene* titleScene = nullptr;
StageSelectScene* stageSelectScene = nullptr;
GameScene* gameScene = nullptr;
ClearScene* clearScene = nullptr;

// --- シーン制御用の関数プロトタイプ宣言 ---
void ChangeScene();
void UpdateScene();
void DrawScene();

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	KamataEngine::Initialize(L"LE2B_14_タカナガ_ダイキ_ウツロイ");
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// --- 最初のシーンをタイトルに設定 ---
	scene = Scene::kTitle;
	titleScene = new TitleScene();
	titleScene->Initialize();

	// メインループ
	while (true) {
		if (KamataEngine::Update()) {
			break;
		}

		UpdateScene();
		ChangeScene();

		dxCommon->PreDraw();
		DrawScene();
		dxCommon->PostDraw();
	}

	// --- 終了処理 ---
	delete titleScene;
	delete stageSelectScene;
	delete gameScene;
	delete clearScene;
	KamataEngine::Finalize();
	return 0;
}

void UpdateScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Update();
		break;
	case Scene::kStageSelect:
		stageSelectScene->Update();
		break;
	case Scene::kGame:
		gameScene->Update();
		break;
	case Scene::kClear:
		clearScene->Update();
		break;
	}
}

void DrawScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Draw();
		break;
	case Scene::kStageSelect:
		stageSelectScene->Draw();
		break;
	case Scene::kGame:
		gameScene->Draw();
		break;
	case Scene::kClear:
		clearScene->Draw();
		break;
	}
}

void ChangeScene() {
	switch (scene) {
	case Scene::kTitle:
		if (titleScene->IsFinished()) {
			scene = Scene::kStageSelect;
			delete titleScene;
			titleScene = nullptr;
			stageSelectScene = new StageSelectScene();
			stageSelectScene->Initialize();
		}
		break;

	case Scene::kStageSelect:
		if (stageSelectScene->IsFinished()) {
			scene = Scene::kGame;
			g_currentStage = stageSelectScene->GetSelectedStage();
			delete stageSelectScene;
			stageSelectScene = nullptr;
			gameScene = new GameScene();
			gameScene->Initialize(g_currentStage);
		}
		break;

	case Scene::kGame:
		if (gameScene->IsFinished()) {
			if (gameScene->IsCleared()) {
				GameScene::ClearResult result = gameScene->GetClearResult();
				delete gameScene;
				gameScene = nullptr;

				switch (result) {
				case GameScene::ClearResult::kNextStage:
					g_currentStage++;
					if (g_currentStage > kMaxStages) {
						// 最終ステージクリアなら全体クリア画面へ
						scene = Scene::kClear;
						clearScene = new ClearScene();
						clearScene->Initialize();
					} else {
						// 次のステージへ
						scene = Scene::kGame;
						gameScene = new GameScene();
						gameScene->Initialize(g_currentStage);
					}
					break;

				case GameScene::ClearResult::kStageSelect:
					scene = Scene::kStageSelect;
					stageSelectScene = new StageSelectScene();
					stageSelectScene->Initialize();
					break;

				case GameScene::ClearResult::kTitle:
					scene = Scene::kTitle;
					titleScene = new TitleScene();
					titleScene->Initialize();
					break;
				}

			} else {
				// 死亡した場合はステージセレクトに戻る
				scene = Scene::kStageSelect;
				delete gameScene;
				gameScene = nullptr;
				stageSelectScene = new StageSelectScene();
				stageSelectScene->Initialize();
			}
		}
		break;

	case Scene::kClear:
		if (clearScene->IsFinished()) {
			scene = Scene::kTitle;
			delete clearScene;
			clearScene = nullptr;
			titleScene = new TitleScene();
			titleScene->Initialize();
		}
		break;
	}
}