#pragma once
#include "KamataEngine.h"
#include "MapChipField.h"
#include "mathStruct.h"
#include <stdint.h>

class MapChipField;
class Enemy;

class Player {
public:
	enum class PlayerColor { kNormal, kRed, kGreen, kBlue };

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
	PlayerColor GetCurrentColor() const { return currentColor_; }
	int GetKeyCount() const { return keyCount_; } // ★鍵の数を取得

	// --- セッター ---
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	// --- 鍵の操作 ---
	void AddKey(); // ★鍵を1つ増やす
	void UseKey(); // ★鍵を1つ消費する

	void OnCollision(const Enemy* enemy);

private:
	// ...(Behavior, AttackPhase, etc)...
	enum class Behavior { kRoot, kAttack, kUnknown };
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

	struct CollisionMapInfo {
		bool ceilingCollision = false;
		bool groundCollision = false;
		bool wallCollision = false;
		Vector3 moveVector;
	};
	enum Corner { kRightBottom, kLeftBottom, kRightTop, kLeftTop, kNumCorner };

	bool isMapChipSolid(MapChipField::MapChipType type);
	void ResolveStuckState();
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

	PlayerColor currentColor_ = PlayerColor::kNormal;
	ObjectColor objectColor_;
	int keyCount_ = 0;                 // ★boolからintに変更 (鍵の所持数)
	static const int kMaxKeyCount = 3; // ★鍵の最大所持数

	// ...(定数)...
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