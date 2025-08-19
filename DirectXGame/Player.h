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
	void Initialize(Model* model, Model* modelAttackRight, Model* modelAttackLeft, uint32_t textureHandle, Camera* camera, const Vector3& position);
	void Update();
	void Draw();

	// --- ゲッター ---
	const Vector3& GetVelocity() const { return velocity_; }
	const WorldTransform& GetWorldTransform() const { return worldTransform_; }
	Vector3 GetWorldPosition() const;
	AABB GetAABB();
	bool IsDead() const { return isDead_; }
	bool IsAttack() const { return behavior_ == Behavior::kAttack; }

	// --- セッター ---
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	// --- 衝突処理 ---
	void OnCollision(const Enemy* enemy);

	enum class PlayerColor {
		kNormal, // 通常色
		kRed,    // 赤
		kBlue,   // 青
	};

private:
	// --- 振る舞い（ビヘイビア） ---
	enum class Behavior {
		kRoot,
		kAttack,
		kUnknown,
	};
	Behavior behavior_ = Behavior::kRoot;
	Behavior behaviorRequest_ = Behavior::kUnknown;

	enum class AttackPhase {
		kAnticipation,
		kDash,
		kFollowThrough,
	};
	AttackPhase attackPhase_;

	void BehaviorRootInitialize();
	void BehaviorRootUpdate();
	void BehaviorAttackInitialize();
	void BehaviorAttackUpdate();

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
	WorldTransform worldTransformAttack_;
	Model* model_ = nullptr;
	Model* modelAttackRight_ = nullptr;
	Model* modelAttackLeft_ = nullptr;
	uint32_t textureHandleAttack_ = 0;
	Camera* camera_ = nullptr;
	Vector3 velocity_ = {};
	bool isDead_ = false;
	bool onGround_ = true;
	enum class LRDirection { kRight, kLeft };
	LRDirection lrdirection_ = LRDirection::kRight;
	float turnFIrstRotationY_ = 0.0f;
	float turnTimer_ = 0.0f;
	MapChipField* mapChipField_ = nullptr;

	//---色に関する変数---
	PlayerColor currentColor_ = PlayerColor::kNormal;
	ObjectColor objectColor_; // モデルの色

	// --- 定数 ---
	static inline const float kAcceleration = 0.03f;
	static inline const float kAttenuation = 0.02f;
	static inline const float kLimitRunSpeed = 0.5f;
	static inline const float kTimeTurn = 0.3f;
	static inline const float kGravityAcceleration = 0.01f;
	static inline const float kLimitFallSpeed = 0.3f;
	static inline const float kJumpAcceleration = 0.5f;
	static inline const float kAttenuationLanding = 0.05f;
	static inline const float kAttenuationWall = 0.2f;
	static inline const float kWidth = 1.8f;
	static inline const float kHeight = 1.8f;
	static inline const uint32_t kAttackAnticipationDuration = 10;
	static inline const uint32_t kAttackDashDuration = 10;
	static inline const uint32_t kAttackFollowThroughDuration = 30;
	static inline const Vector3 kAttackVelocity = {0.8f, 0.0f, 0.0f};
};