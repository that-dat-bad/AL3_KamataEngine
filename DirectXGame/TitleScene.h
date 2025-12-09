#pragma once
#include "IScene.h"
#include "KamataEngine.h"
#include "TitleLogo.h"
#include"TitleGuide.h"
#include <list>

class TitleScene : public IScene {
public:
	~TitleScene();

	// 初期化
	void Initialize() override;

	// 更新
	std::optional<SceneID> Update() override;

	// 描画
	void Draw() override;

private:
	uint32_t textureHandle_ = 0;

	// 3Dモデルデータ
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera camera_;

	KamataEngine::WorldTransform worldTransform_;

	// キーボード入力
	KamataEngine::Input* input_ = nullptr;

	// デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// デバッグカメラ有効
	bool isDebugCameraActive_ = false;

	// フェーズ別更新処理
	std::optional<SceneID> UpdateFadeIn();
	std::optional<SceneID> UpdateMain();
	std::optional<SceneID> UpdateFadeOut();

	// フェード用スプライト
	KamataEngine::Sprite* fadeSprite_ = nullptr;
	// フェード用テクスチャハンドル (1x1の白画像で代用)
	uint32_t fadeTextureHandle_ = 0;

	//タイトルロゴの3Dモデル
	TitleLogo* logo_ = nullptr;
	KamataEngine::Vector3 logoPosition_ = {0.0f, 0.0f, -45.0f};
	// 誘導の3Dモデル
	TitleGuide* guide_ = nullptr;
	KamataEngine::Vector3 guidePosition_ = {-1.0f, -2.0f, -40.0f};

	//天球
	KamataEngine::Model* skydomeModel_ = nullptr;
	KamataEngine::WorldTransform skydomeTransform_;
};