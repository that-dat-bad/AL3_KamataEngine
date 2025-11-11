#pragma once
#include "IScene.h"
#include "KamataEngine.h"
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

	std::optional<SceneID> UpdateFadeIn();
	std::optional<SceneID> UpdateMain();
	std::optional<SceneID> UpdateFadeOut();
};