#pragma once
#include "KamataEngine.h"
#include "PlayerBullet.h"
#include "PlayerMissile.h"
#include <list>
#include <stdint.h>

class Player {
public:
	~Player();
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera);
	void Update();
	void Draw();
	void SetBulletModel(KamataEngine::Model* model) { bulletModel_ = model; };
	void SetMissileModel(KamataEngine::Model* model) { missileModel_ = model; }

	// リスト取得
	const std::list<PlayerBullet*>& GetBullets() const { return bullets_; }
	const std::list<PlayerMissile*>& GetMissiles() const { return missiles_; }

	// 座標・回転取得
	KamataEngine::Vector3 GetWorldPosition() const { return worldTransform_.translation_; }
	const KamataEngine::Vector3& GetRotation() const { return worldTransform_.rotation_; }

	// HP関連のゲッター
	int GetHP() const { return hp_; }
	int GetMaxHP() const { return kMaxHP_; }
	bool IsDead() const { return isDead_; }

	//残機数の取得	
	int GetLives() const { return lives_; }

	// 衝突処理
	void OnCollision();

	// ミサイル発射
	void FireMissile(Enemy* target);

private:
	void Attack();
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* bulletModel_ = nullptr;
	KamataEngine::Model* missileModel_ = nullptr;

	KamataEngine::Camera* camera_ = nullptr;
	KamataEngine::Input* input_ = nullptr;
	std::list<PlayerBullet*> bullets_;
	std::list<PlayerMissile*> missiles_;

	// HPパラメータ
	static const int kMaxHP_ = 10;
	int hp_ = kMaxHP_;
	bool isDead_ = false;

	//残機
	static const int kDefaultLives_ = 3; // 初期残機数
	int lives_ = kDefaultLives_;

	// 無敵時間タイマー
	int invincibleTimer_ = 0;
};