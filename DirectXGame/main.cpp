#include "KamataEngine.h"
#include <Windows.h>


using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// エンジンの初期化
	KamataEngine::Initialize(L"LE2B_14_タカナガ_ダイキ_AL3");

	// メインループ
	while (true) {

		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}
	}

	//エンジンの終了
	KamataEngine::Finalize();
	return 0;
}
