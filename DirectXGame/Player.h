#pragma once
#include "KamataEngine.h"
#include "PlayerBullet.h"
#include <list>
#include <stdint.h>

class Player {
public:
	/// <summary>
	/// デストラクタ
	/// </summary>
	~Player();

	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera);

	void Update();

	void Draw();
	
	void SetBulletModel(KamataEngine::Model* model) { bulletModel_ = model; };

private:
	/// <summary>
	/// 攻撃
	/// </summary>
	void Attack();

	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;
	// 3Dモデルデータ
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* bulletModel_ = nullptr;

	KamataEngine::Camera* camera_ = nullptr;

	// キーボード入力
	KamataEngine::Input* input_ = nullptr;

	// 弾
	std::list<PlayerBullet*> bullets_;
};