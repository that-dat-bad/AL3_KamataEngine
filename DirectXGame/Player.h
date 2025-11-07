#pragma once
#include "KamataEngine.h"
#include "PlayerBullet.h"
#include <stdint.h>

class Player {
public:
	void Initialize(KamataEngine::Model* model, uint32_t textureHandle, KamataEngine::Camera* camera);

	void Update();

	void Draw();

private:
	/// <summary>
	/// 攻撃
	/// </summary>
	void Attack();

	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;
	// 3Dモデルデータ
	KamataEngine::Model* model_ = nullptr;
	// テクスチャハンドル
	uint32_t textureHandle_ = 0;

	KamataEngine::Camera* camera_ = nullptr;

	// キーボード入力
	KamataEngine::Input* input_ = nullptr;

	// 弾
	PlayerBullet* bullet_ = nullptr;
};