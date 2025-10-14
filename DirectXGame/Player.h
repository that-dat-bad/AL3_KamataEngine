#pragma once
#include "KamataEngine.h"
#include "MapChipField.h"
#include "bulletManager.h"
#include <cassert>
#include <stdint.h>
using namespace KamataEngine;

class MapChipField;

class Player {
public:
	enum class PlayerState {
		kGround,
		kJump,
		kApexSpin,
		kFall,
	};

	struct CollisionMapInfo {
		bool ceilingCollision = false;
		bool groundCollision = false;
		bool wallCollision = false;
		Vector3 moveVector;
	};

	enum Corner { kRightBottom, kLeftBottom, kRightTop, kLeftTop, kNumCorner };

void Initialize(Model* model, Camera* camera, const Vector3& position, BulletManager* bulletManager);
	void Update();
	void Draw();
	const Vector3& GetVelocity() const { return velocity_; }
	const WorldTransform& GetWorldTransform() const { return worldTransform_; }
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }
	void MapCollider(CollisionMapInfo& info);
	void CeilingCollision(CollisionMapInfo& info);
	Vector3 CornerPosition(const Vector3& center, Corner corner);
	void ResultReflectToMove(const CollisionMapInfo& info);
	void OnCeilingCollision(const CollisionMapInfo& info);
	void GroundCollision(CollisionMapInfo& info);
	void RightCollision(CollisionMapInfo& info);
	void LeftCollision(CollisionMapInfo& info);
	void OnWallCollision(const CollisionMapInfo& info);
	bool IsOnGround();

private:
	void StartApexSpin();

	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	Camera* camera_ = nullptr;
	Vector3 velocity_ = {};

	static inline const float kAcceleration = 0.03f;
	static inline const float kAttenuation = 0.02f;
	static inline const float kLimitRunSpeed = 0.5f;
	static inline const float kGravityAcceleration = 0.02f;
	static inline const float kLimitFallSpeed = 0.5f;
	static inline const float kJumpVelocity = 0.6f;
	static inline const float kAirShotRecoil = 0.7f;
	static inline const int kApexSpinDuration = 30;

	static inline const float kTimeTurn = 0.3f;
	static inline const float kBlank = 0.02f;
	static inline const float kAttenuationLanding = 0.05f;
	static inline const float kAttenuationWall = 0.2f;

	enum class LRDirection {
		kRight,
		kLeft,
	};
	LRDirection lrdirection_ = LRDirection::kRight;

	float turnFIrstRotationY_ = 0.0f;
	float turnTimer_ = 0.0f;

	PlayerState state_ = PlayerState::kGround;
	int apexSpinTimer_ = 0;

	bool canAirShot_ = true;

	MapChipField* mapChipField_ = nullptr;
	BulletManager* bulletManager_ = nullptr;
	static inline const float kWidth = 1.8f;
	static inline const float kHeight = 1.8f;
};