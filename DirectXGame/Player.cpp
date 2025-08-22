#include "Player.h"
#include "mathStruct.h"
#include <algorithm>
#include <cmath>
#include <numbers>

void Player::Initialize(Model* model, Model* modelAttackRight, Model* modelAttackLeft, uint32_t textureHandle, Camera* camera, const Vector3& position) {
	assert(model);
	assert(modelAttackRight);
	assert(modelAttackLeft);
	model_ = model;
	modelAttackRight_ = modelAttackRight;
	modelAttackLeft_ = modelAttackLeft;
	textureHandleAttack_ = textureHandle;
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

void Player::MapCollider(CollisionMapInfo& info) {
	CeilingCollision(info);
	GroundCollision(info);
	LeftCollision(info);
	RightCollision(info);
}

void Player::CeilingCollision(CollisionMapInfo& info) {
	if (info.moveVector.y <= 0.0f) {
		return;
	}
	Vector3 positionsNew[kNumCorner];
	for (int i = 0; i < kNumCorner; ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveVector, static_cast<Corner>(i));
	}
	bool hit = false;
	float ceilingBlockBottomY = FLT_MAX;
	MapChipField::IndexSet indexSetLeft = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	MapChipField::MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSetLeft.xIndex, indexSetLeft.yIndex);
	MapChipField::MapChipType mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSetLeft.xIndex, indexSetLeft.yIndex - 1);
	if (isMapChipSolid(mapChipType) && !isMapChipSolid(mapChipTypeNext)) {
		hit = true;
		MapChipField::Rect blockRect = mapChipField_->GetRectByIndex(indexSetLeft.xIndex, indexSetLeft.yIndex);
		ceilingBlockBottomY = (std::min)(ceilingBlockBottomY, blockRect.bottom);
	}
	MapChipField::IndexSet indexSetRight = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSetRight.xIndex, indexSetRight.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSetRight.xIndex, indexSetRight.yIndex - 1);
	if (isMapChipSolid(mapChipType) && !isMapChipSolid(mapChipTypeNext)) {
		hit = true;
		MapChipField::Rect blockRect = mapChipField_->GetRectByIndex(indexSetRight.xIndex, indexSetRight.yIndex);
		ceilingBlockBottomY = (std::min)(ceilingBlockBottomY, blockRect.bottom);
	}
	if (hit) {
		info.ceilingCollision = true;
		float playerTopY = worldTransform_.translation_.y + (kHeight / 2.0f);
		float newMoveY = ceilingBlockBottomY - playerTopY;
		info.moveVector.y = (std::max)(0.0f, newMoveY);
	}
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {
	Vector3 offsetTable[kNumCorner] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0.0f},
        {-kWidth / 2.0f, -kHeight / 2.0f, 0.0f},
        {+kWidth / 2.0f, +kHeight / 2.0f, 0.0f},
        {-kWidth / 2.0f, +kHeight / 2.0f, 0.0f}
    };
	return center + offsetTable[static_cast<uint32_t>(corner)];
}

void Player::ResultReflectToMove(const CollisionMapInfo& info) { worldTransform_.translation_ += info.moveVector; }

void Player::OnCeilingCollision(const CollisionMapInfo& info) {
	if (info.ceilingCollision) {
		velocity_.y = 0.0f;
	}
}

void Player::GroundCollision(CollisionMapInfo& info) {
	if (info.moveVector.y >= 0.0f) {
		return;
	}
	Vector3 positionsNew[kNumCorner];
	for (int i = 0; i < kNumCorner; ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveVector, static_cast<Corner>(i));
	}
	bool hit = false;
	float groundBlockTopY = -FLT_MAX;
	MapChipField::IndexSet indexSetLeftNew = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	MapChipField::MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSetLeftNew.xIndex, indexSetLeftNew.yIndex);
	MapChipField::MapChipType mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSetLeftNew.xIndex, indexSetLeftNew.yIndex - 1);
	if (isMapChipSolid(mapChipType) && !isMapChipSolid(mapChipTypeNext)) {
		hit = true;
		MapChipField::Rect blockRect = mapChipField_->GetRectByIndex(indexSetLeftNew.xIndex, indexSetLeftNew.yIndex);
		groundBlockTopY = (std::max)(groundBlockTopY, blockRect.top);
	}
	MapChipField::IndexSet indexSetRightNew = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSetRightNew.xIndex, indexSetRightNew.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSetRightNew.xIndex, indexSetRightNew.yIndex - 1);
	if (isMapChipSolid(mapChipType) && !isMapChipSolid(mapChipTypeNext)) {
		hit = true;
		MapChipField::Rect blockRect = mapChipField_->GetRectByIndex(indexSetRightNew.xIndex, indexSetRightNew.yIndex);
		groundBlockTopY = (std::max)(groundBlockTopY, blockRect.top);
	}
	if (hit) {
		info.groundCollision = true;
		float playerBottomY = worldTransform_.translation_.y - (kHeight / 2.0f);
		float newMoveY = groundBlockTopY - playerBottomY;
		info.moveVector.y = (std::min)(0.0f, newMoveY);
	}
}

void Player::ToggleOnGround(const CollisionMapInfo& info) {
	if (!onGround_) {
		if (info.groundCollision) {
			onGround_ = true;
			velocity_.x *= (1.0f - kAttenuationLanding);
			velocity_.y = 0.0f;
		}
	} else {
		if (velocity_.y > 0.0f) {
			onGround_ = false;
			return;
		}

		const float kGroundCheckEpsilon = 0.1f;
		Vector3 checkPosOffset = {0.0f, -kGroundCheckEpsilon, 0.0f};
		Vector3 posLeftBottom = CornerPosition(worldTransform_.translation_ + checkPosOffset, kLeftBottom);
		Vector3 posRightBottom = CornerPosition(worldTransform_.translation_ + checkPosOffset, kRightBottom);

		MapChipField::IndexSet indexSetLeft = mapChipField_->GetMapChipIndexSetByPosition(posLeftBottom);
		MapChipField::MapChipType typeLeft = mapChipField_->GetMapChipTypeByIndex(indexSetLeft.xIndex, indexSetLeft.yIndex);
		MapChipField::MapChipType typeAboveLeft = mapChipField_->GetMapChipTypeByIndex(indexSetLeft.xIndex, indexSetLeft.yIndex - 1);
		bool isGroundUnderLeft = isMapChipSolid(typeLeft) && !isMapChipSolid(typeAboveLeft);

		MapChipField::IndexSet indexSetRight = mapChipField_->GetMapChipIndexSetByPosition(posRightBottom);
		MapChipField::MapChipType typeRight = mapChipField_->GetMapChipTypeByIndex(indexSetRight.xIndex, indexSetRight.yIndex);
		MapChipField::MapChipType typeAboveRight = mapChipField_->GetMapChipTypeByIndex(indexSetRight.xIndex, indexSetRight.yIndex - 1);
		bool isGroundUnderRight = isMapChipSolid(typeRight) && !isMapChipSolid(typeAboveRight);

		if (!isGroundUnderLeft && !isGroundUnderRight) {
			onGround_ = false;
		}
	}
}

void Player::RightCollision(CollisionMapInfo& info) {
	if (info.moveVector.x <= 0.0f) {
		return;
	}
	const float kCollisionCheckMargin = 0.01f;
	bool hit = false;
	float blockLeftX = FLT_MAX;
	Corner corners[2] = {kRightTop, kRightBottom};
	for (int i = 0; i < 2; ++i) {
		Vector3 checkPos = CornerPosition(worldTransform_.translation_, corners[i]);
		checkPos.y += (corners[i] == kRightTop) ? -kCollisionCheckMargin : kCollisionCheckMargin;
		Vector3 checkPosNew = checkPos + info.moveVector;
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(checkPosNew);
		MapChipField::MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
		MapChipField::MapChipType mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);
		if (isMapChipSolid(mapChipType) && !isMapChipSolid(mapChipTypeNext)) {
			hit = true;
			MapChipField::Rect blockRect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			blockLeftX = (std::min)(blockLeftX, blockRect.left);
		}
	}
	if (hit) {
		info.wallCollision = true;
		float playerRightX = worldTransform_.translation_.x + (kWidth / 2.0f);
		info.moveVector.x = playerRightX > blockLeftX ? blockLeftX - playerRightX : 0.0f;
	}
}

void Player::LeftCollision(CollisionMapInfo& info) {
	if (info.moveVector.x >= 0.0f) {
		return;
	}
	const float kCollisionCheckMargin = 0.01f;
	bool hit = false;
	float blockRightX = -FLT_MAX;
	Corner corners[2] = {kLeftTop, kLeftBottom};
	for (int i = 0; i < 2; ++i) {
		Vector3 checkPos = CornerPosition(worldTransform_.translation_, corners[i]);
		checkPos.y += (corners[i] == kLeftTop) ? -kCollisionCheckMargin : kCollisionCheckMargin;
		Vector3 checkPosNew = checkPos + info.moveVector;
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(checkPosNew);
		MapChipField::MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
		MapChipField::MapChipType mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);
		if (isMapChipSolid(mapChipType) && !isMapChipSolid(mapChipTypeNext)) {
			hit = true;
			MapChipField::Rect blockRect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			blockRightX = (std::max)(blockRightX, blockRect.right);
		}
	}
	if (hit) {
		info.wallCollision = true;
		float playerLeftX = worldTransform_.translation_.x - (kWidth / 2.0f);
		info.moveVector.x = playerLeftX < blockRightX ? blockRightX - playerLeftX : 0.0f;
	}
}

void Player::OnWallCollision(const CollisionMapInfo& info) {
	if (info.wallCollision) {
		velocity_.x *= (1.0f - kAttenuationWall);
	}
}