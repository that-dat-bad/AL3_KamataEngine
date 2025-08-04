#pragma once
#include "KamataEngine.h"
#include "MapChipField.h"
#include "mathStruct.h"
#include <stdint.h>

using namespace KamataEngine;

class MapChipField;
class Enemy;

class Player {
public:
	// --- 基本関数 ---
	void Initialize(Model* model, Camera* camera, const Vector3& position);
	void Update();
	void Draw();

	// --- ゲッター ---
	const Vector3& GetVelocity() const { return velocity_; }
	const WorldTransform& GetWorldTransform() const { return worldTransform_; }
	Vector3 GetWorldPosition();
	AABB GetAABB();
	bool IsDead() const { return isDead_; }

	// --- セッター ---
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	// --- 衝突処理 ---
	void OnCollision(const Enemy* enemy);

private:
	// --- 振る舞い（ビヘイビア） ---
	enum class Behavior {
		kRoot,    // 通常状態
		kAttack,  // 攻撃中
		kUnknown, // 不明（リクエストなし）
	};
	Behavior behavior_ = Behavior::kRoot;
	Behavior behaviorRequest_ = Behavior::kUnknown;

	// --- 各ビヘイビアの処理関数 ---
	void BehaviorRootInitialize();
	void BehaviorRootUpdate();
	void BehaviorAttackInitialize();
	void BehaviorAttackUpdate();

	// 攻撃ギミックの時間経過カウンター
	uint32_t attackParameter_ = 0;

	// --- マップとの衝突判定用の内部関数・構造体 ---
	struct CollisionMapInfo {
		bool ceilingCollision = false;
		bool groundCollision = false;
		bool wallCollision = false;
		Vector3 moveVector;
	};
	enum Corner { kRightBottom, kLeftBottom, kRightTop, kLeftTop, kNumCorner };
	void MapCollider(CollisionMapInfo& info);
	void CeilingCollision(CollisionMapInfo& info);
	Vector3 CornerPosition(const Vector3& center, Corner corner);
	void ResultReflectToMove(const CollisionMapInfo& info);
	void OnCeilingCollision(const CollisionMapInfo& info);
	void GroundCollision(CollisionMapInfo& info);
	void ToggleOnGround(const CollisionMapInfo& info);
	void RightCollision(CollisionMapInfo& info);
	void LeftCollision(CollisionMapInfo& info);
	void OnWallCollision(const CollisionMapInfo& info);

	// --- 物理挙動・状態に関する変数 ---
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	Camera* camera_ = nullptr;
	Vector3 velocity_ = {};
	bool isDead_ = false;
	bool onGround_ = true;
	enum class LRDirection { kRight, kLeft };
	LRDirection lrdirection_ = LRDirection::kRight;
	float turnFIrstRotationY_ = 0.0f;
	float turnTimer_ = 0.0f;
	MapChipField* mapChipField_ = nullptr;

	// --- 定数 ---
	static inline const float kAcceleration = 0.03f;
	static inline const float kAttenuation = 0.02f;
	static inline const float kLimitRunSpeed = 0.5f;
	static inline const float kTimeTurn = 0.3f;
	static inline const float kGravityAcceleration = 0.01f;
	static inline const float kLimitFallSpeed = 0.3f;
	static inline const float kJumpAcceleration = 0.5f;
	static inline const float kBlank = 0.02f;
	static inline const float kAttenuationLanding = 0.05f;
	static inline const float kAttenuationWall = 0.2f;
	static inline const float kWidth = 1.8f;
	static inline const float kHeight = 1.8f;
	static inline const float kAttackDashSpeed = 0.8f;
};