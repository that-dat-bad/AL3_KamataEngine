#pragma once
#include "EnemyBullet.h"
#include "KamataEngine.h"
#include <list>
#include <stdint.h>

// Playerクラスがあることを前方宣言
class Player;

class Enemy {
public:
	~Enemy();

	// ★変更: velocityを受け取るように変更
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity);

	void Update();
	void Draw(const KamataEngine::Camera& camera);

	bool IsDead() const { return isDead_; }

	void SetBulletModel(KamataEngine::Model* model) { bulletModel_ = model; };

	// プレイヤーの情報をセットする関数
	void SetPlayer(Player* player) { player_ = player; }

	KamataEngine::Vector3 GetWorldPosition() const { return worldTransform_.translation_; }
	const std::list<EnemyBullet*>& GetBullets() const { return bullets_; }
	void OnCollision();

private:
	void UpdateApproach();
	void UpdateLeave();
	void Fire();

	using StateFunction = void (Enemy::*)();
	StateFunction stateFunction_ = nullptr;

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* bulletModel_ = nullptr;

	// ★追加: 速度
	KamataEngine::Vector3 velocity_;

	bool isDead_ = false;

	std::list<EnemyBullet*> bullets_;

	// プレイヤーへのポインタ
	Player* player_ = nullptr;
};