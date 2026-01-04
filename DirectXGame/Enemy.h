#pragma once
#include "EnemyBullet.h"
#include "KamataEngine.h"
#include <list>
#include <string>

class Player;

//敵のタイプ
enum class EnemyType {
	TypeA, // 雑魚
	TypeB, // 硬い
};

//攻撃パターン
enum class AttackPattern {
	None,   // 撃たない
	Normal, // 通常弾
	Homing, // 追尾弾
};

class Enemy {
public:
	~Enemy();

	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity, const std::string& typeStr, const std::string& patternStr);

	void Update();
	void Draw(const KamataEngine::Camera& camera);

	void SetPlayer(Player* player) { player_ = player; }
	void SetBulletModel(KamataEngine::Model* model) { bulletModel_ = model; }

	const std::list<EnemyBullet*>& GetBullets() const { return bullets_; }
	KamataEngine::Vector3 GetWorldPosition() const { return worldTransform_.translation_; }

	void OnCollision(int damage);
	bool IsDead() const { return isDead_; }

private:
	void UpdateApproach();
	void UpdateLeave();
	void Fire();

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* bulletModel_ = nullptr;
	Player* player_ = nullptr;
	KamataEngine::Vector3 velocity_;
	std::list<EnemyBullet*> bullets_;

	void (Enemy::*stateFunction_)() = nullptr;


	int hp_ = 0;
	bool isDead_ = false;
	EnemyType type_ = EnemyType::TypeA;
	AttackPattern attackPattern_ = AttackPattern::None;
};