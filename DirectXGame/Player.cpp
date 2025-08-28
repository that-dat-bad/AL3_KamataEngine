#include "Player.h"
#include "mathStruct.h"
#include <algorithm>
#include <cmath>
#include <numbers>

void Player::Initialize(Model* model, Camera* camera, const Vector3& position) {
	assert(model);
	model_ = model;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransformAttack_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	objectColor_.Initialize();

	BehaviorRootInitialize();
}

void Player::Update() {
	// ビヘイビアの遷移処理
	if (behaviorRequest_ != Behavior::kUnknown) {
		behavior_ = behaviorRequest_;
		switch (behavior_) {
		case Behavior::kRoot:
		default:
			BehaviorRootInitialize();
			break;
		case Behavior::kAttack:
			BehaviorAttackInitialize();
			break;
		}
		behaviorRequest_ = Behavior::kUnknown;
	}

	// ビヘイビアごとの更新処理
	switch (behavior_) {
	case Behavior::kRoot:
	default:
		BehaviorRootUpdate();
		break;
	case Behavior::kAttack:
		BehaviorAttackUpdate();
		break;
	}

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Player::BehaviorRootInitialize() {
	// 通常行動の初期化
}

void Player::BehaviorRootUpdate() {
	// 色変更と、それに伴うめり込み解決処理
	PlayerColor preColor = currentColor_;
	if (Input::GetInstance()->TriggerKey(DIK_1)) {
		currentColor_ = PlayerColor::kNormal;
	}
	if (Input::GetInstance()->TriggerKey(DIK_2)) {
		currentColor_ = PlayerColor::kRed;
	}
	if (Input::GetInstance()->TriggerKey(DIK_3)) {
		currentColor_ = PlayerColor::kGreen;
	}
	if (Input::GetInstance()->TriggerKey(DIK_4)) {
		currentColor_ = PlayerColor::kBlue;
	}
	// 色が変更された瞬間、めり込みをチェック・解決する
	if (preColor != currentColor_) {
		ResolveStuckState();
	}

	// 現在の色に応じて、ObjectColorの色を設定する
	switch (currentColor_) {
	case PlayerColor::kRed:
		objectColor_.SetColor({1.0f, 0.2f, 0.2f, 1.0f});
		break;
	case PlayerColor::kGreen:
		objectColor_.SetColor({0.2f, 1.0f, 0.2f, 1.0f});
		break;
	case PlayerColor::kBlue:
		objectColor_.SetColor({0.2f, 0.2f, 1.0f, 1.0f});
		break;
	case PlayerColor::kNormal:
	default:
		objectColor_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
		break;
	}

	// 1. 入力と物理演算
	if (onGround_) {
		velocity_.y = 0;
		if (Input::GetInstance()->PushKey(DIK_UP)) {
			velocity_.y = kJumpAcceleration;
			onGround_ = false;
		}
		if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
			if (velocity_.x < 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}
			velocity_.x += kAcceleration;
			if (lrdirection_ != LRDirection::kRight) {
				lrdirection_ = LRDirection::kRight;
				turnFIrstRotationY_ = worldTransform_.rotation_.y;
				turnTimer_ = kTimeTurn;
			}
		} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
			if (velocity_.x > 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}
			velocity_.x -= kAcceleration;
			if (lrdirection_ != LRDirection::kLeft) {
				lrdirection_ = LRDirection::kLeft;
				turnFIrstRotationY_ = worldTransform_.rotation_.y;
				turnTimer_ = kTimeTurn;
			}
		} else {
			velocity_.x *= (1.0f - kAttenuation);
		}
	} else {
		velocity_.y -= kGravityAcceleration;
	}
	velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
	velocity_.y = std::clamp(velocity_.y, -kLimitFallSpeed, kJumpAcceleration);

	// 2. 衝突計算の準備
	CollisionMapInfo collisionMapInfo;
	collisionMapInfo.moveVector = velocity_;

	// 3. マップとの衝突判定と移動量の補正
	MapCollider(collisionMapInfo);

	// 4. 補正された移動量で座標を更新
	ResultReflectToMove(collisionMapInfo);

	// 5. ギミックとの当たり判定
	AABB playerAABB = GetAABB();
	MapChipField::IndexSet indexMin = mapChipField_->GetMapChipIndexSetByPosition(playerAABB.min);
	MapChipField::IndexSet indexMax = mapChipField_->GetMapChipIndexSetByPosition(playerAABB.max);
	for (uint32_t y = indexMax.yIndex; y <= indexMin.yIndex; ++y) {
		for (uint32_t x = indexMin.xIndex; x <= indexMax.xIndex; ++x) {
			MapChipField::MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(x, y);
			switch (mapChipType) {
			case MapChipField::MapChipType::kCurtain_SetRed:
				currentColor_ = PlayerColor::kRed;
				goto GimmickCheckEnd;
			case MapChipField::MapChipType::kCurtain_SetGreen:
				currentColor_ = PlayerColor::kGreen;
				goto GimmickCheckEnd;
			case MapChipField::MapChipType::kCurtain_SetBlue:
				currentColor_ = PlayerColor::kBlue;
				goto GimmickCheckEnd;
			}
		}
	}
GimmickCheckEnd:;

	// 6. 衝突後の状態更新
	OnCeilingCollision(collisionMapInfo);
	ToggleOnGround(collisionMapInfo);
	OnWallCollision(collisionMapInfo);

	// 7. 旋回制御
	if (turnTimer_ > 0.0f) {
		turnTimer_ -= 1.0f / 60.0f;
		float destinationRotationYTable[] = {
		    std::numbers::pi_v<float> / 2.0f,
		    std::numbers::pi_v<float> * 3.0f / 2.0f,
		};
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrdirection_)];
		float progress = 1.0f - (turnTimer_ / kTimeTurn);
		worldTransform_.rotation_.y = turnFIrstRotationY_ + (destinationRotationY - turnFIrstRotationY_) * progress;
	}

	// 8. 攻撃リクエスト
	if (Input::GetInstance()->PushKey(DIK_A)) {
		behaviorRequest_ = Behavior::kAttack;
	}
}

void Player::BehaviorAttackInitialize() {
	attackParameter_ = 0;
	attackPhase_ = AttackPhase::kAnticipation;
	velocity_ = {};
}

void Player::BehaviorAttackUpdate() {
	Vector3 velocity{};
	// ...(省略)... 元のコードと同じ
}

void Player::Draw() {
	Model::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	model_->Draw(worldTransform_, *camera_, &objectColor_);

	if (behavior_ == Behavior::kAttack) {

		if (lrdirection_ == LRDirection::kRight) {
			modelAttackRight_->Draw(worldTransformAttack_, *camera_);
		} else {
			modelAttackLeft_->Draw(worldTransformAttack_, *camera_);
		}
	}

	Model::PostDraw();
}

void Player::OnCollision(const Enemy* enemy) {
	if (IsAttack()) {
		return;
	}
	(void)enemy;
	isDead_ = true;
}

Vector3 Player::GetWorldPosition() const {
	Vector3 worldPos;
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];
	return worldPos;
}

AABB Player::GetAABB() {
	Vector3 center = GetWorldPosition();
	AABB aabb;
	aabb.min.x = center.x - kWidth / 2.0f;
	aabb.max.x = center.x + kWidth / 2.0f;
	aabb.min.y = center.y - kHeight / 2.0f;
	aabb.max.y = center.y + kHeight / 2.0f;
	aabb.min.z = center.z - kWidth / 2.0f;
	aabb.max.z = center.z + kWidth / 2.0f;
	return aabb;
}

bool Player::isMapChipSolid(MapChipField::MapChipType type) {
	if (type == MapChipField::MapChipType::kLockedDoor) {
		return keyCount_ <= 0;
	}
	if (currentColor_ == PlayerColor::kNormal) {
		return (type == MapChipField::MapChipType::kBlock);
	}
	switch (type) {
	case MapChipField::MapChipType::kBlock:
		return true;
	case MapChipField::MapChipType::kBlock_Red:
		return (currentColor_ == PlayerColor::kRed);
	case MapChipField::MapChipType::kBlock_Green:
		return (currentColor_ == PlayerColor::kGreen);
	case MapChipField::MapChipType::kBlock_Blue:
		return (currentColor_ == PlayerColor::kBlue);
	default:
		return false;
	}
}

void Player::ResolveStuckState() {
	MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_);
	MapChipField::MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (isMapChipSolid(mapChipType)) {
		MapChipField::Rect blockRect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		AABB playerAABB = GetAABB();
		float pushUpDistance = blockRect.top - playerAABB.min.y;
		float pushDownDistance = blockRect.bottom - playerAABB.max.y;
		if (pushUpDistance < std::abs(pushDownDistance)) {
			worldTransform_.translation_.y += pushUpDistance;
		} else {
			worldTransform_.translation_.y += pushDownDistance;
		}
		velocity_.y = 0;
	}
}

#pragma region 当たり判定
void Player::MapCollider(CollisionMapInfo& info) {
	CeilingCollision(info);
	GroundCollision(info);
	RightCollision(info);
	LeftCollision(info);
}

void Player::CeilingCollision(CollisionMapInfo& info) {
	// 上向きの移動でない場合は処理しない
	const float kEpsilon = 0.001f;
	if (info.moveVector.y <= kEpsilon) {
		return;
	}

	Vector3 posLeftTopNew = CornerPosition(worldTransform_.translation_, kLeftTop) + info.moveVector;
	Vector3 posRightTopNew = CornerPosition(worldTransform_.translation_, kRightTop) + info.moveVector;

	bool hit = false;
	float ceilingBlockBottomY = FLT_MAX;

	// 左上の角の判定
	if (CheckCeilingCollisionAtPosition(posLeftTopNew, ceilingBlockBottomY)) {
		hit = true;
	}

	// 右上の角の判定
	if (CheckCeilingCollisionAtPosition(posRightTopNew, ceilingBlockBottomY)) {
		hit = true;
	}

	if (hit) {
		info.ceilingCollision = true;
		float playerTopY = worldTransform_.translation_.y + (kHeight / 2.0f);
		const float kCollisionMargin = 0.001f;
		float newMoveY = ceilingBlockBottomY - playerTopY - kCollisionMargin;
		info.moveVector.y = (std::min)(info.moveVector.y, newMoveY);
	}
}

bool Player::CheckCeilingCollisionAtPosition(const Vector3& position, float& minCeilingY) {
	MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(position);

	// 境界チェック（簡易版）
	MapChipField::MapChipType currentType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	// 現在位置が固体でない場合は衝突なし
	if (!isMapChipSolid(currentType)) {
		return false;
	}

	// 下のチップを確認
	MapChipField::MapChipType typeBelowCurrent = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);

	// 天井判定：現在が固体かつ下が非固体の場合
	if (isMapChipSolid(currentType) && !isMapChipSolid(typeBelowCurrent)) {
		MapChipField::Rect blockRect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		minCeilingY = (std::min)(minCeilingY, blockRect.bottom);
		return true;
	}

	return false;
}

void Player::GroundCollision(CollisionMapInfo& info) {
	// 下向きの移動でない場合は処理しない
	const float kEpsilon = 0.001f;
	if (info.moveVector.y >= -kEpsilon) {
		return;
	}

	Vector3 posLeftBottomNew = CornerPosition(worldTransform_.translation_, kLeftBottom) + info.moveVector;
	Vector3 posRightBottomNew = CornerPosition(worldTransform_.translation_, kRightBottom) + info.moveVector;

	bool hit = false;
	float groundBlockTopY = -FLT_MAX;

	// 左下の角の判定
	if (CheckGroundCollisionAtPosition(posLeftBottomNew, groundBlockTopY)) {
		hit = true;
	}

	// 右下の角の判定
	if (CheckGroundCollisionAtPosition(posRightBottomNew, groundBlockTopY)) {
		hit = true;
	}

	if (hit) {
		info.groundCollision = true;
		float playerBottomY = worldTransform_.translation_.y - (kHeight / 2.0f);
		const float kCollisionMargin = 0.001f;
		float newMoveY = groundBlockTopY - playerBottomY + kCollisionMargin;
		info.moveVector.y = (std::max)(info.moveVector.y, newMoveY);
	}
}

bool Player::CheckGroundCollisionAtPosition(const Vector3& position, float& maxGroundY) {
	MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(position);

	MapChipField::MapChipType currentType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	// 現在位置が固体でない場合は衝突なし
	if (!isMapChipSolid(currentType)) {
		return false;
	}

	// 上のチップを確認
	MapChipField::MapChipType typeAboveCurrent = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);

	// 地面判定：現在が固体かつ上が非固体の場合
	if (isMapChipSolid(currentType) && !isMapChipSolid(typeAboveCurrent)) {
		MapChipField::Rect blockRect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		maxGroundY = (std::max)(maxGroundY, blockRect.top);
		return true;
	}

	return false;
}

void Player::RightCollision(CollisionMapInfo& info) {
	const float kEpsilon = 0.001f;
	if (info.moveVector.x <= kEpsilon) {
		return;
	}

	bool hit = false;
	float blockLeftX = FLT_MAX;

	// 角の微調整値（既存の定数を使用しない）
	const float kCornerAdjustment = 0.02f;

	// 右上と右下の角を使用して壁判定
	Corner corners[2] = {kRightTop, kRightBottom};

	for (int i = 0; i < 2; ++i) {
		Vector3 checkPos = CornerPosition(worldTransform_.translation_, corners[i]);

		// 角の微調整（内側に少しずらして引っかかりを防ぐ）
		checkPos.y += (corners[i] == kRightTop) ? -kCornerAdjustment : kCornerAdjustment;

		Vector3 checkPosNew = checkPos + info.moveVector;

		if (CheckWallCollisionAtPosition(checkPosNew, true, blockLeftX)) {
			hit = true;
		}
	}

	if (hit) {
		info.wallCollision = true;
		float playerRightX = worldTransform_.translation_.x + (kWidth / 2.0f);
		const float kCollisionMargin = 0.001f;
		float newMoveX = blockLeftX - playerRightX - kCollisionMargin;
		info.moveVector.x = (std::min)(info.moveVector.x, newMoveX);
	}
}

void Player::LeftCollision(CollisionMapInfo& info) {
	const float kEpsilon = 0.001f;
	if (info.moveVector.x >= -kEpsilon) {
		return;
	}

	bool hit = false;
	float blockRightX = -FLT_MAX;

	// 角の微調整値
	const float kCornerAdjustment = 0.02f;

	// 左上と左下の角を使用して壁判定
	Corner corners[2] = {kLeftTop, kLeftBottom};

	for (int i = 0; i < 2; ++i) {
		Vector3 checkPos = CornerPosition(worldTransform_.translation_, corners[i]);

		// 角の微調整（内側に少しずらして引っかかりを防ぐ）
		checkPos.y += (corners[i] == kLeftTop) ? -kCornerAdjustment : kCornerAdjustment;

		Vector3 checkPosNew = checkPos + info.moveVector;

		if (CheckWallCollisionAtPosition(checkPosNew, false, blockRightX)) {
			hit = true;
		}
	}

	if (hit) {
		info.wallCollision = true;
		float playerLeftX = worldTransform_.translation_.x - (kWidth / 2.0f);
		const float kCollisionMargin = 0.001f;
		float newMoveX = blockRightX - playerLeftX + kCollisionMargin;
		info.moveVector.x = (std::max)(info.moveVector.x, newMoveX);
	}
}

bool Player::CheckWallCollisionAtPosition(const Vector3& position, bool checkingRightWall, float& wallX) {
	MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(position);

	MapChipField::MapChipType currentType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	// 現在位置が固体でない場合は衝突なし
	if (!isMapChipSolid(currentType)) {
		return false;
	}

	// 隣接するチップを確認
	int neighborX = checkingRightWall ? indexSet.xIndex - 1 : indexSet.xIndex + 1;
	MapChipField::MapChipType neighborType = mapChipField_->GetMapChipTypeByIndex(neighborX, indexSet.yIndex);

	// 壁判定：現在が固体かつ隣が非固体の場合
	if (isMapChipSolid(currentType) && !isMapChipSolid(neighborType)) {
		MapChipField::Rect blockRect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);

		if (checkingRightWall) {
			wallX = (std::min)(wallX, blockRect.left);
		} else {
			wallX = (std::max)(wallX, blockRect.right);
		}
		return true;
	}

	return false;
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {
	Vector3 offsetTable[kNumCorner] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0.0f}, // kRightBottom
	    {-kWidth / 2.0f, -kHeight / 2.0f, 0.0f}, // kLeftBottom
	    {+kWidth / 2.0f, +kHeight / 2.0f, 0.0f}, // kRightTop
	    {-kWidth / 2.0f, +kHeight / 2.0f, 0.0f}  // kLeftTop
	};
	return center + offsetTable[static_cast<uint32_t>(corner)];
}

void Player::ToggleOnGround(const CollisionMapInfo& info) {
	const float kEpsilon = 0.001f;

	if (!onGround_) {
		if (info.groundCollision) {
			onGround_ = true;
			velocity_.x *= (1.0f - kAttenuationLanding);
			velocity_.y = 0.0f;
		}
	} else {
		if (velocity_.y > kEpsilon) {
			onGround_ = false;
			return;
		}

		// 地面チェックのためのオフセット（既存の定数値を使用）
		const float kGroundCheckEpsilon = 0.1f;
		Vector3 checkPosOffset = {0.0f, -kGroundCheckEpsilon, 0.0f};
		Vector3 posLeftBottom = CornerPosition(worldTransform_.translation_ + checkPosOffset, kLeftBottom);
		Vector3 posRightBottom = CornerPosition(worldTransform_.translation_ + checkPosOffset, kRightBottom);

		bool isGroundUnderLeft = CheckGroundUnderPosition(posLeftBottom);
		bool isGroundUnderRight = CheckGroundUnderPosition(posRightBottom);

		if (!isGroundUnderLeft && !isGroundUnderRight) {
			onGround_ = false;
		}
	}
}

bool Player::CheckGroundUnderPosition(const Vector3& position) {
	MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(position);

	MapChipField::MapChipType currentType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (!isMapChipSolid(currentType)) {
		return false;
	}

	// 上のチップを確認
	MapChipField::MapChipType typeAbove = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);

	return isMapChipSolid(currentType) && !isMapChipSolid(typeAbove);
}

void Player::ResultReflectToMove(const CollisionMapInfo& info) { worldTransform_.translation_ += info.moveVector; }

void Player::OnCeilingCollision(const CollisionMapInfo& info) {
	if (info.ceilingCollision) {
		velocity_.y = 0.0f;
	}
}

void Player::OnWallCollision(const CollisionMapInfo& info) {
	if (info.wallCollision) {
		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

#pragma endregion

void Player::AddKey() {
	if (keyCount_ < kMaxKeyCount) {
		keyCount_++;
	}
}

void Player::UseKey() {
	if (keyCount_ > 0) {
		keyCount_--;
	}
}
