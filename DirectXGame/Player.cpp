#include "Player.h"
#include "mathStruct.h"
#include <algorithm>
#include <numbers>

void Player::Initialize(Model* model, Model* modelAttack, Camera* camera, const Vector3& position) {
	assert(model);
	assert(modelAttack);
	model_ = model;
	modelAttack_ = modelAttack;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransformAttack_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

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

	// 5. 衝突後の状態更新
	OnCeilingCollision(collisionMapInfo);
	ToggleOnGround(collisionMapInfo);
	OnWallCollision(collisionMapInfo);

	// 6. 旋回制御
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

	// 7. 攻撃リクエスト
	if (Input::GetInstance()->PushKey(DIK_A)) {
		behaviorRequest_ = Behavior::kAttack;
	}
}

void Player::BehaviorAttackInitialize() {
	// 攻撃開始時は「溜め」から
	attackPhase_ = AttackPhase::kAnticipation;
	attackParameter_ = 0;
	// 攻撃開始時に速度をゼロにする
	velocity_ = {};
}

void Player::BehaviorAttackUpdate() {
	// 攻撃用のローカルな速度変数
	Vector3 velocity{};

	// 攻撃のサブフェーズに応じて処理を分岐
	switch (attackPhase_) {
	case AttackPhase::kAnticipation: // 溜め動作
	{
		float t = static_cast<float>(attackParameter_) / kAttackAnticipationDuration;
		// 横に縮んで縦に伸びる (Squash & Stretch)
		worldTransform_.scale_.z = EaseOut(1.0f, 0.3f, t);
		worldTransform_.scale_.y = EaseOut(1.0f, 1.6f, t);

		// 溜め時間が終わったら突進へ移行
		if (attackParameter_ >= kAttackAnticipationDuration) {
			attackPhase_ = AttackPhase::kDash;
			attackParameter_ = 0; // カウンターをリセット
		}
	} break;

	case AttackPhase::kDash: // 突進動作
	{
		// 突進中だけ移動させる
		if (lrdirection_ == LRDirection::kRight) {
			velocity = kAttackVelocity;
		} else {
			velocity = {-kAttackVelocity.x, kAttackVelocity.y, kAttackVelocity.z};
		}

		float t = static_cast<float>(attackParameter_) / kAttackDashDuration;
		// 横に伸びて縦に縮む
		worldTransform_.scale_.z = EaseOut(0.3f, 1.3f, t);
		worldTransform_.scale_.y = EaseIn(1.6f, 0.7f, t);
		worldTransform_.translation_ += velocity;

		// 突進時間が終わったら余韻へ移行
		if (attackParameter_ >= kAttackDashDuration) {
			attackPhase_ = AttackPhase::kFollowThrough;
			attackParameter_ = 0; // カウンターをリセット
		}
	} break;

	case AttackPhase::kFollowThrough: // 余韻動作
	{
		float t = static_cast<float>(attackParameter_) / kAttackFollowThroughDuration;
		// 通常サイズに戻る
		worldTransform_.scale_.z = EaseOut(1.3f, 1.0f, t);
		worldTransform_.scale_.y = EaseIn(0.7f, 1.0f, t);

		// 余韻時間が終わったら通常状態へ戻るリクエストを出す
		if (attackParameter_ >= kAttackFollowThroughDuration) {
			behaviorRequest_ = Behavior::kRoot;
		}
	} break;
	}

	// 攻撃中のパラメータを加算
	attackParameter_++;

	worldTransformAttack_.translation_ = worldTransform_.translation_;
	worldTransformAttack_.rotation_ = worldTransform_.rotation_;
	worldTransformAttack_.matWorld_ = MakeAffineMatrix(worldTransformAttack_.scale_, worldTransformAttack_.rotation_, worldTransformAttack_.translation_);
	worldTransformAttack_.TransferMatrix();
	
	// 突進の移動量をvelocity_に反映させる
	velocity_ = velocity;
}

void Player::Draw() {
	Model::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	model_->Draw(worldTransform_, *camera_);
	// 攻撃中の場合のみ、エフェクトを描画する
	if (behavior_ == Behavior::kAttack) {
		modelAttack_->Draw(worldTransformAttack_, *camera_);
	}
	Model::PostDraw();

}

void Player::OnCollision(const Enemy* enemy) {
	// 攻撃中はダメージを受けない
	if (IsAttack()) {
		return;
	}
	(void)enemy;
	isDead_ = true;
}

Vector3 Player::GetWorldPosition() {
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
		MapChipField::IndexSet indexSetRight = mapChipField_->GetMapChipIndexSetByPosition(posRightBottom);
		MapChipField::MapChipType typeRight = mapChipField_->GetMapChipTypeByIndex(indexSetRight.xIndex, indexSetRight.yIndex);

		if (typeLeft != MapChipField::MapChipType::kBlock && typeRight != MapChipField::MapChipType::kBlock) {
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