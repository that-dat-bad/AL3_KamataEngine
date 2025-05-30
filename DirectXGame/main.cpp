#include "KamataEngine.h"
#include <Windows.h>
#include"GameScene.h"

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// エンジンの初期化
	KamataEngine::Initialize(L"LE2B_14_タカナガ_ダイキ_AL3");
	//DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	
	// ゲームシーンのインスタンス生成
	GameScene* gameScene = new GameScene();

	// ゲームシーンの初期化
	gameScene->Initialize();

	// メインループ
	while (true) {

		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		// ゲームシーンの更新
		gameScene->Update();

		//描画開始
		dxCommon->PreDraw();



		// ゲームシーンの描画
		gameScene->Draw();


		//描画終了
		dxCommon->PostDraw();

	}
	delete gameScene;
	// ゲームシーンの終了処理
	gameScene = nullptr;
	//エンジンの終了
	KamataEngine::Finalize();
	return 0;
}
