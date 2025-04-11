#pragma once
#include "KamataEngine.h"
class GameScene {
public:

	// コンストラクタ
	GameScene();

	// デストラクタ
	~GameScene();

	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	//テクスチャハンドル
	uint32_t textureHandle_ = 0;

	//スプライト
	KamataEngine::Sprite* sprite_ = nullptr;
};
