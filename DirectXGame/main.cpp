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
	kStageSelect, // ★ステージセレクトシーンを追加
	kGame,
};

// 現在のシーン
Scene scene = Scene::kUnknown;
// ステージの総数
const int kMaxStages = 3;
// 現在のステージ番号を管理する変数
static int g_currentStage = 1;

// 各シーンのインスタンスへのポインタ
TitleScene* titleScene = nullptr;
StageSelectScene* stageSelectScene = nullptr;
GameScene* gameScene = nullptr;

// --- シーン制御用の関数プロトタイプ宣言 ---
void ChangeScene();
void UpdateScene();
void DrawScene();

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	KamataEngine::Initialize(L"LE2B_14_タカナガ_ダイキ_AL3");
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
			g_currentStage = stageSelectScene->GetSelectedStage(); // 選択したステージ番号を取得
			delete stageSelectScene;
			stageSelectScene = nullptr;
			gameScene = new GameScene();
			gameScene->Initialize(g_currentStage);
		}
		break;

	case Scene::kGame:
		if (gameScene->IsFinished()) {
			// クリアしたかどうかで分岐
			if (gameScene->IsCleared()) {
				g_currentStage++; // 次のステージへ
				if (g_currentStage > kMaxStages) {
					// 最終ステージクリアならタイトルへ
					scene = Scene::kTitle;
					delete gameScene;
					gameScene = nullptr;
					titleScene = new TitleScene();
					titleScene->Initialize();
				} else {
					// 次のステージへ
					delete gameScene;
					gameScene = new GameScene();
					gameScene->Initialize(g_currentStage);
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
	}
}