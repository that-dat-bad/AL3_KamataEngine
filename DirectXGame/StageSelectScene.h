#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

class StageSelectScene {
public:
	void Initialize();
	void Update();
	void Draw();

	bool IsFinished() const { return isFinished_; }
	int GetSelectedStage() const { return selectedStage_; }

private:
	// ステージの総数
	static const int kMaxStages = 3;

	// 選択中のステージ番号 (1-indexed)
	int selectedStage_ = 1;

	// 終了フラグ
	bool isFinished_ = false;

	// 画像表示用のスプライト
	Sprite* sprites_[kMaxStages] = {};
};