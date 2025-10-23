#include "Player.h"
#include "mathStruct.h"
#include <algorithm>
#include <numbers>

void Player::Initialize(Model* model, Model* arrowModel, Camera* camera, const Vector3& position, BulletManager* bulletManager) {
	assert(model);
	assert(arrowModel);
	assert(camera);
	assert(bulletManager);

	model_ = model;
	arrowModel_ = arrowModel;
	camera_ = camera;
	bulletManager_ = bulletManager;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	arrowWorldTransform_.Initialize();
	arrowWorldTransform_.scale_ = {2.0f, 2.0f, 2.0f};
	playingState_ = PlayingState::kPlaying;
}

void Player::Update() {
	if (isDead_) {
		playingState_ = PlayingState::kGameOver;
		return;
	}

	switch (state_) {
	case PlayerState::kGround:
		velocity_.y = 0;
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			velocity_.y = kJumpVelocity;
			stompJumpAvailable_ = false;
			canAirShot_ = true;
			state_ = PlayerState::kJump;
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
		break;

	case PlayerState::kJump:
		if (stompJumpAvailable_ && Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			StartApexSpin();
			stompJumpAvailable_ = false;
			break;
		}
		velocity_.y -= kGravityAcceleration;
		if (velocity_.y <= 0.0f) {
			StartApexSpin();
		}
		break;

	case PlayerState::kApexSpin: {
		if (canAirShot_ && Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			float angle = worldTransform_.rotation_.z;
			Vector3 bulletVelocity = {cosf(angle) * 0.8f, sinf(angle) * 0.8f, 0.0f};
			bulletManager_->SpawnBullet(worldTransform_.translation_, bulletVelocity, this);
			velocity_.x = -cosf(angle) * kAirShotRecoil;
			velocity_.y = -sinf(angle) * kAirShotRecoil;
			canAirShot_ = false;
			stompJumpAvailable_ = false;
			worldTransform_.rotation_.z = 0;
			state_ = PlayerState::kFall;
			break;
		}

		apexSpinTimer_--;
		{
			float progress = 1.0f - (static_cast<float>(apexSpinTimer_) / kApexSpinDuration);
			worldTransform_.rotation_.z = -progress * 2.0f * std::numbers::pi_v<float>;
		}
		if (apexSpinTimer_ <= 0) {
			worldTransform_.rotation_.z = 0;
			state_ = PlayerState::kFall;
		}

		arrowWorldTransform_.rotation_.y = worldTransform_.rotation_.y;
		arrowWorldTransform_.rotation_.z = worldTransform_.rotation_.z + std::numbers::pi_v<float>;
		float offsetDistance = 2.0f;
		Vector3 offsetDirection = {cosf(arrowWorldTransform_.rotation_.z), sinf(arrowWorldTransform_.rotation_.z), 0.0f};
		arrowWorldTransform_.translation_ = worldTransform_.translation_ + offsetDirection * offsetDistance;
		arrowWorldTransform_.matWorld_ = MakeAffineMatrix(arrowWorldTransform_.scale_, arrowWorldTransform_.rotation_, arrowWorldTransform_.translation_);
		arrowWorldTransform_.TransferMatrix();

		break;
	}

	case PlayerState::kFall:
		if (stompJumpAvailable_ && Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			StartApexSpin();
			stompJumpAvailable_ = false; 
			break;                       
		}

		velocity_.y -= kGravityAcceleration;

		break;
	}

	velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
	velocity_.y = std::clamp(velocity_.y, -kLimitFallSpeed, kJumpVelocity);

	CollisionMapInfo collisionMapInfo;
	collisionMapInfo.moveVector = velocity_;

	MapCollider(collisionMapInfo);
	ResultReflectToMove(collisionMapInfo);

	OnCeilingCollision(collisionMapInfo);

	if (state_ == PlayerState::kFall && collisionMapInfo.groundCollision) {
		state_ = PlayerState::kGround;
		stompJumpAvailable_ = false;
	}

	if (state_ == PlayerState::kGround) {
		if (!IsOnGround()) {
			state_ = PlayerState::kFall;
		}
	}
	OnWallCollision(collisionMapInfo);

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

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Player::Draw() {
	Model::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	model_->Draw(worldTransform_, *camera_);
	if (state_ == PlayerState::kApexSpin) {
		arrowModel_->Draw(arrowWorldTransform_, *camera_);
	}
	Model::PostDraw();
}

void Player::OnEnemyStomp() {
	velocity_.y = kJumpVelocity * 0.8f;
	ResetAirAction();
	state_ = PlayerState::kJump;
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
	MapChipField::MapChipType mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSetLeft.xIndex, indexSetLeft.yIndex + 1);
	if (mapChipType == MapChipField::MapChipType::kBlock && mapChipTypeNext != MapChipField::MapChipType::kBlock) {
		hit = true;
		MapChipField::Rect blockRect = mapChipField_->GetRectByIndex(indexSetLeft.xIndex, indexSetLeft.yIndex);
		ceilingBlockBottomY = (std::min)(ceilingBlockBottomY, blockRect.bottom);
	}
	MapChipField::IndexSet indexSetRight = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSetRight.xIndex, indexSetRight.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSetRight.xIndex, indexSetRight.yIndex + 1);
	if (mapChipType == MapChipField::MapChipType::kBlock && mapChipTypeNext != MapChipField::MapChipType::kBlock) {
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
		if (state_ == PlayerState::kJump) {
			StartApexSpin();
		}
	}
}

void Player::GroundCollision(CollisionMapInfo& info) {
	if (info.moveVector.y >= 0.0f) {
		return;
	}
	MapChipField::IndexSet indexSetLeftNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kLeftBottom));
	MapChipField::IndexSet indexSetRightNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kRightBottom));
	Vector3 positionsNew[kNumCorner];
	for (int i = 0; i < kNumCorner; ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveVector, static_cast<Corner>(i));
	}
	bool hit = false;
	float groundBlockTopY = -FLT_MAX;
	MapChipField::IndexSet indexSetLeftNew = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	MapChipField::MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSetLeftNew.xIndex, indexSetLeftNew.yIndex);
	MapChipField::MapChipType mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSetLeftNew.xIndex, indexSetLeftNew.yIndex - 1);
	if (mapChipType == MapChipField::MapChipType::kBlock && mapChipTypeNext != MapChipField::MapChipType::kBlock && indexSetLeftNow.yIndex != indexSetLeftNew.yIndex) {
		hit = true;
		MapChipField::Rect blockRect = mapChipField_->GetRectByIndex(indexSetLeftNew.xIndex, indexSetLeftNew.yIndex);
		groundBlockTopY = (std::max)(groundBlockTopY, blockRect.top);
	}
	MapChipField::IndexSet indexSetRightNew = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSetRightNew.xIndex, indexSetRightNew.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSetRightNew.xIndex, indexSetRightNew.yIndex - 1);
	if (mapChipType == MapChipField::MapChipType::kBlock && mapChipTypeNext != MapChipField::MapChipType::kBlock && indexSetRightNow.yIndex != indexSetRightNew.yIndex) {
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
		if (mapChipType == MapChipField::MapChipType::kBlock && mapChipTypeNext != MapChipField::MapChipType::kBlock) {
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
		if (mapChipType == MapChipField::MapChipType::kBlock && mapChipTypeNext != MapChipField::MapChipType::kBlock) {
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

bool Player::IsOnGround() {
	const float kGroundCheckEpsilon = 0.1f;
	Vector3 checkPosOffset = {0.0f, -kGroundCheckEpsilon, 0.0f};
	Vector3 posLeftBottom = CornerPosition(worldTransform_.translation_ + checkPosOffset, kLeftBottom);
	Vector3 posRightBottom = CornerPosition(worldTransform_.translation_ + checkPosOffset, kRightBottom);
	MapChipField::IndexSet indexSetLeft = mapChipField_->GetMapChipIndexSetByPosition(posLeftBottom);
	MapChipField::MapChipType typeLeft = mapChipField_->GetMapChipTypeByIndex(indexSetLeft.xIndex, indexSetLeft.yIndex);
	MapChipField::IndexSet indexSetRight = mapChipField_->GetMapChipIndexSetByPosition(posRightBottom);
	MapChipField::MapChipType typeRight = mapChipField_->GetMapChipTypeByIndex(indexSetRight.xIndex, indexSetRight.yIndex);
	if (typeLeft == MapChipField::MapChipType::kBlock || typeRight == MapChipField::MapChipType::kBlock) {
		return true;
	}
	return false;
}

void Player::StartApexSpin() {
	velocity_ = {};
	apexSpinTimer_ = kApexSpinDuration;
	state_ = PlayerState::kApexSpin;
}

AABB Player::GetAABB() {
	Vector3 size = {kWidth, kHeight, 1.0f};
	Vector3 worldPos = worldTransform_.translation_;
	return {
	    {worldPos.x - size.x / 2.0f, worldPos.y - size.y / 2.0f, worldPos.z - size.z / 2.0f},
        {worldPos.x + size.x / 2.0f, worldPos.y + size.y / 2.0f, worldPos.z + size.z / 2.0f}
    };
}