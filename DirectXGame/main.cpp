#include "GameScene.h"
#include "KamataEngine.h"
#include <Windows.h>
#include"SceneManager.h"
using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// エンジンの初期化
	KamataEngine::Initialize(L"LE2B_14_タカナガ_ダイキ_AL4");

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// ImGuiManagerインスタンスの取得
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();

	// ゲームシーンのインスタンス生成
	SceneManager* sceneManager = new SceneManager();
	sceneManager->Initialize();
	// メインループ
	while (true) {

		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		// ImGuiフレーム開始
		imguiManager->Begin();

		sceneManager->Update();

		// ImGuiフレーム終了
		imguiManager->End();

		// 描画開始
		dxCommon->PreDraw();

		sceneManager->Draw();

		// ImGui描画
		imguiManager->Draw();

		// 描画終了
		dxCommon->PostDraw();
	}

	// エンジンの終了
	KamataEngine::Finalize();
	return 0;
}