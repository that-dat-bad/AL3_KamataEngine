#include "Player.h"
#include "mathStruct.h"
#include <algorithm>
#include <numbers>

void Player::Initialize(Model* model, Camera* camera, const Vector3& position) {
	// nullポインタチェック
	assert(model);

	// 引数として受け取ったデータをメンバ変数に記録する
	model_ = model;
	camera_ = camera;
	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
}

void Player::Update() {
	// --- 1. 入力と物理演算 ---
	if (onGround_) {
		// ▼ 接地状態の処理
		velocity_.y = 0; // 地面にいるときはY速度を0に

		// ジャンプ入力
		if (Input::GetInstance()->PushKey(DIK_UP)) {
			velocity_.y = kJumpAcceleration;
			onGround_ = false; // すぐに空中状態へ
		}

		// 左右移動
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
			// 入力がない場合は摩擦で減速
			velocity_.x *= (1.0f - kAttenuation);
		}
	} else {
		// ▼ 空中状態の処理 (重力)
		velocity_.y -= kGravityAcceleration;
	}
	// 速度制限
	velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
	velocity_.y = std::clamp(velocity_.y, -kLimitFallSpeed, kJumpAcceleration);

	// --- 2. 衝突計算の準備 ---
	CollisionMapInfo collisionMapInfo;
	collisionMapInfo.moveVector = velocity_; // フレームレート依存の簡易的な移動量

	// --- 3. マップとの衝突判定と移動量の補正 ---
	MapCollider(collisionMapInfo);

	// --- 4. 補正された移動量で座標を更新 ---
	ResultReflectToMove(collisionMapInfo);

	// --- 5. 衝突後の状態更新 ---
	OnCeilingCollision(collisionMapInfo); // 天井に当たった時の処理
	ToggleOnGround(collisionMapInfo);     // 接地状態を更新 (重要)
	OnWallCollision(collisionMapInfo);

	// --- 6. 旋回制御と行列更新 ---
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
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Model::PreDraw(dxCommon->GetCommandList());
	model_->Draw(worldTransform_, *camera_);
	Model::PostDraw();
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

	// 左上点の判定
	MapChipField::IndexSet indexSetLeft = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);

	MapChipField::MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSetLeft.xIndex, indexSetLeft.yIndex);
	MapChipField::MapChipType mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSetLeft.xIndex, indexSetLeft.yIndex + 1);

	if (mapChipType == MapChipField::MapChipType::kBlock && mapChipTypeNext != MapChipField::MapChipType::kBlock) {

		hit = true;
		MapChipField::Rect blockRect = mapChipField_->GetRectByIndex(indexSetLeft.xIndex, indexSetLeft.yIndex);
		ceilingBlockBottomY = (std::min)(ceilingBlockBottomY, blockRect.bottom);
	}

	// 右上点の判定
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
	// 中心座標にオフセットを加算してコーナー座標を計算
	return center + offsetTable[static_cast<uint32_t>(corner)];
}

void Player::ResultReflectToMove(const CollisionMapInfo& info) {
	// 移動
	worldTransform_.translation_ += info.moveVector;
}

void Player::OnCeilingCollision(const CollisionMapInfo& info) {
	if (info.ceilingCollision) {
		velocity_.y = 0.0f; // 上昇中の速度をリセット
	}
}

void Player::GroundCollision(CollisionMapInfo& info) {
	if (info.moveVector.y >= 0.0f) {
		return;
	}

	// 移動前の足元座標のマップチップインデックスを取得
	MapChipField::IndexSet indexSetLeftNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kLeftBottom));
	MapChipField::IndexSet indexSetRightNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kRightBottom));

	Vector3 positionsNew[kNumCorner];
	for (int i = 0; i < kNumCorner; ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveVector, static_cast<Corner>(i));
	}

	bool hit = false;
	float groundBlockTopY = -FLT_MAX;

	// 左下点の判定
	MapChipField::IndexSet indexSetLeftNew = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	MapChipField::MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSetLeftNew.xIndex, indexSetLeftNew.yIndex);
	MapChipField::MapChipType mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSetLeftNew.xIndex, indexSetLeftNew.yIndex - 1);

	// Yインデックスが移動前後で変わったか
	if (mapChipType == MapChipField::MapChipType::kBlock && mapChipTypeNext != MapChipField::MapChipType::kBlock && indexSetLeftNow.yIndex != indexSetLeftNew.yIndex) {
		hit = true;
		MapChipField::Rect blockRect = mapChipField_->GetRectByIndex(indexSetLeftNew.xIndex, indexSetLeftNew.yIndex);
		groundBlockTopY = (std::max)(groundBlockTopY, blockRect.top);
	}

	// 右下点の判定
	MapChipField::IndexSet indexSetRightNew = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSetRightNew.xIndex, indexSetRightNew.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSetRightNew.xIndex, indexSetRightNew.yIndex - 1);

	// Yインデックスが移動前後で変わったか
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
	// 現在、空中状態の場合の処理
	if (!onGround_) {
		// 着地フラグがtrueなら、接地状態へ移行する
		if (info.groundCollision) {
			onGround_ = true;
			// 着地時にX速度を減衰させる
			velocity_.x *= (1.0f - kAttenuationLanding);
			// Y速度をゼロにする（地面にめり込まないように）
			velocity_.y = 0.0f;
		}
	}
	// 現在、地上状態の場合の処理
	else {
		// ジャンプなどでY速度が正になったら、空中状態へ移行
		if (velocity_.y > 0.0f) {
			onGround_ = false;
			return; // これ以降の地上チェックは不要
		}

		// --- 落下判定 ---
		// 足元に地面があるかチェックする
		// わずかに下に判定点をずらして調べる
		const float kGroundCheckEpsilon = 0.1f;
		Vector3 checkPosOffset = {0.0f, -kGroundCheckEpsilon, 0.0f};

		// チェックする足元の2点の座標
		Vector3 posLeftBottom = CornerPosition(worldTransform_.translation_ + checkPosOffset, kLeftBottom);
		Vector3 posRightBottom = CornerPosition(worldTransform_.translation_ + checkPosOffset, kRightBottom);

		// 各点のマップチップ情報を取得
		MapChipField::IndexSet indexSetLeft = mapChipField_->GetMapChipIndexSetByPosition(posLeftBottom);
		MapChipField::MapChipType typeLeft = mapChipField_->GetMapChipTypeByIndex(indexSetLeft.xIndex, indexSetLeft.yIndex);

		MapChipField::IndexSet indexSetRight = mapChipField_->GetMapChipIndexSetByPosition(posRightBottom);
		MapChipField::MapChipType typeRight = mapChipField_->GetMapChipTypeByIndex(indexSetRight.xIndex, indexSetRight.yIndex);

		// 左右両方の足元にブロックがなければ、落下開始
		if (typeLeft != MapChipField::MapChipType::kBlock && typeRight != MapChipField::MapChipType::kBlock) {
			onGround_ = false; // 空中状態に切り替える
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

	// 壁接触による減速
	if (info.wallCollision) {
		velocity_.x *= (1.0f - kAttenuationWall);
	}
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

void Player::OnCollision(const Enemy* enemy) {
	(void)enemy;
	// 仮処理

}
