#pragma once
#include "KamataEngine.h"
#include "PlayerBullet.h"
#include <list>
#include <stdint.h>

class Player {
public:
	~Player();
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera);
	void Update();
	void Draw();
	void SetBulletModel(KamataEngine::Model* model) { bulletModel_ = model; };

	// ★追加: 弾リストの取得
	const std::list<PlayerBullet*>& GetBullets() const { return bullets_; }

	// ★追加: 座標取得
	KamataEngine::Vector3 GetWorldPosition() const { return worldTransform_.translation_; }

	// ★追加: 衝突処理 (ダメージなど)
	void OnCollision() {
		// ここにHPを減らす処理や死亡処理を書く
		// 今回は仮でログ出力のみ、あるいは何もしない
	}

private:
	void Attack();
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* bulletModel_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	KamataEngine::Input* input_ = nullptr;
	std::list<PlayerBullet*> bullets_;
};