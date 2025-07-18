#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

class TitleScene {
public:
	// 初期化
	void Initialize();
	// 更新
	void Update();
	// 描画
	void Draw();

	// 終了したかを取得
	bool IsFinished() const { return finished_; }

private:
	// 終了フラグ
	bool finished_ = false;
};