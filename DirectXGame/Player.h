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
	struct CollisionMapInfo {
		// 天井衝突フラグ
		bool ceilingCollision = false;
		// 着地フラグ
		bool groundCollision = false;
		// 壁接触フラグ
		bool wallCollision = false;
		// 移動量
		Vector3 moveVector;
	};

	enum Corner {
		kRightBottom,
		kLeftBottom,
		kRightTop,
		kLeftTop,

		kNumCorner

	};

	// 初期化
	void Initialize(Model* model, Camera* camera, const Vector3& position);
	// 更新
	void Update();
	// 描画
	void Draw();

	const Vector3& GetVelocity() const { return velocity_; }

	// ワールド変換データの取得
	const WorldTransform& GetWorldTransform() const { return worldTransform_; }

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

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

	Vector3 GetWorldPosition();

	AABB GetAABB();

	void OnCollision(const Enemy* enemy);

	bool IsDead() const { return isDead_; }

private:
	// ワールド変換データ
	WorldTransform worldTransform_;
	// 3Dモデルデータ
	Model* model_ = nullptr;

	Camera* camera_ = nullptr;

	Vector3 velocity_ = {};

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

	enum class LRDirection {
		kRight,
		kLeft,
	};
	LRDirection lrdirection_ = LRDirection::kRight;

	// 旋回開始時の角度
	float turnFIrstRotationY_ = 0.0f;

	// 旋回タイマー
	float turnTimer_ = 0.0f;

	// 接地判定
	bool onGround_ = true;

	// マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 1.8f;
	static inline const float kHeight = 1.8f;

	//デスフラグ
	bool isDead_ = false;
};
