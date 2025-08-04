#pragma once
#include "KamataEngine.h"
#include "mathStruct.h"
#include <numbers>

using namespace KamataEngine;

class Player;
class GameScene;

class Enemy {
public:
	void Initialize(Model* model, Camera* camera, const Vector3& position);
	void Update();
	void Draw();
	AABB GetAABB();
	Vector3 GetWorldPosition();
	void OnCollision(const Player* player);
	bool IsDead() const { return isDead_; }
	bool IsCollisionDisabled() const { return isCollisionDisabled_; }
	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }

private:
	// --- 振る舞い（ビヘイビア） ---
	enum class Behavior {
		kWalk,  // 歩行
		kDeath, // デス演出
	};
	Behavior behavior_ = Behavior::kWalk;
	Behavior behaviorRequest_ = Behavior::kWalk;

	// --- 各ビヘイビアの処理関数 ---
	void BehaviorWalkUpdate();
	void BehaviorDeathUpdate();

	// --- 状態変数 ---
	bool isDead_ = false;
	bool isCollisionDisabled_ = false;
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	Camera* camera_ = nullptr;
	Vector3 velocity_ = {};
	float walkTimer_ = 0.0f;
	GameScene* gameScene_ = nullptr;

	// --- 定数 ---
	static inline const float kWalkSpeed = -0.1f;
	static inline const float kWalkMotionTime = 1.0f;
	static inline const float kWidth = 1.9f;
	static inline const float kHeight = 1.9f;
	static inline const uint32_t kDeathDuration = 60; // デス演出の時間
};