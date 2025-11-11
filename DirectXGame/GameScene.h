// GameScene.h
#pragma once
#include "Enemy.h"
#include "IScene.h"
#include "KamataEngine.h"
#include "Player.h"
#include <list>
#include <optional>

class GameScene : public IScene {
public:
	~GameScene() override;

	// 初期化
	void Initialize() override;

	// 更新
	std::optional<SceneID> Update() override;

	// 描画
	void Draw() override;

private:
	std::optional<SceneID> UpdateFadeIn();
	std::optional<SceneID> UpdateMain();
	std::optional<SceneID> UpdateFadeOut();

private:
	uint32_t textureHandle_ = 0;

	// --- ★ここから修正 ---
	// 3Dモデルデータ
	KamataEngine::Model* playerModel_ = nullptr;
	KamataEngine::Model* enemyModel_ = nullptr;
	// --- ★ここまで修正 ---

	// カメラ
	KamataEngine::Camera camera_;

	// 自キャラ
	Player* player_ = nullptr;

	KamataEngine::WorldTransform worldTransform_;

	// キーボード入力
	KamataEngine::Input* input_ = nullptr;

	// デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// デバッグカメラ有効
	bool isDebugCameraActive_ = false;

	// 敵
	std::list<Enemy*> enemies_;

	// フェード用スプライト
	KamataEngine::Sprite* fadeSprite_ = nullptr;
	// フェード用テクスチャハンドル (1x1の白画像で代用)
	uint32_t fadeTextureHandle_ = 0;
};