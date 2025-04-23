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


private:

	// テクスチャハンドル
	uint32_t textureHandle_ = 0;

	//サウンドデータハンドル
	uint32_t soundDataHandle_ = 0;

	// 音声データ再生ハンドル
	uint32_t voiceHandle_ = 0;

	//スプライト
	KamataEngine::Sprite* sprite_ = nullptr;

	//3Dモデル
	KamataEngine::Model* model_ = nullptr;

	//ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;

	// カメラ
	KamataEngine::Camera camera_;
};
