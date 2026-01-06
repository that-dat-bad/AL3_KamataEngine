#pragma once
#include "EnemyBullet.h"
#include "EnemyMissile.h"
#include "KamataEngine.h"
#include <list>
#include <string>

class Player;

enum class EnemyType { TypeA, TypeB };
enum class AttackPattern { None, Normal, Homing };

class Enemy {
public:
	~Enemy();

	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity, const std::string& typeStr, const std::string& patternStr);

	void Update();
	void Draw(const KamataEngine::Camera& camera);

	void SetPlayer(Player* player) { player_ = player; }
	void SetBulletModel(KamataEngine::Model* model) { bulletModel_ = model; }
	void SetMissileModel(KamataEngine::Model* model) { missileModel_ = model; }

	const std::list<EnemyBullet*>& GetBullets() const { return bullets_; }
	const std::list<EnemyMissile*>& GetMissiles() const { return missiles_; }

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
	KamataEngine::Model* missileModel_ = nullptr;

	Player* player_ = nullptr;
	KamataEngine::Vector3 velocity_;

	std::list<EnemyBullet*> bullets_;
	std::list<EnemyMissile*> missiles_;

	void (Enemy::*stateFunction_)() = nullptr;

	int hp_ = 0;
	bool isDead_ = false;
	EnemyType type_ = EnemyType::TypeA;
	AttackPattern attackPattern_ = AttackPattern::None;

	// ★追加: 発射間隔管理用タイマー
	int shotTimer_ = 0;
};