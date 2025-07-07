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
	LeftCollision(info);
	RightCollision(info);
	CeilingCollision(info);
	GroundCollision(info);

}

void Player::CeilingCollision(CollisionMapInfo& info) {
	// 上昇中でなければ処理しない
	if (info.moveVector.y <= 0) {
		return;
	}

	// --- チェックポイントの準備 ---
	Vector3 posLeftTopNow = CornerPosition(worldTransform_.translation_, kLeftTop);
	Vector3 posRightTopNow = CornerPosition(worldTransform_.translation_, kRightTop);
	Vector3 posLeftTopNew = CornerPosition(worldTransform_.translation_ + info.moveVector, kLeftTop);
	Vector3 posRightTopNew = CornerPosition(worldTransform_.translation_ + info.moveVector, kRightTop);

	MapChipField::IndexSet indexSetLeftNow = mapChipField_->GetMapChipIndexSetByPosition(posLeftTopNow);
	MapChipField::IndexSet indexSetRightNow = mapChipField_->GetMapChipIndexSetByPosition(posRightTopNow);
	MapChipField::IndexSet indexSetLeftNew = mapChipField_->GetMapChipIndexSetByPosition(posLeftTopNew);
	MapChipField::IndexSet indexSetRightNew = mapChipField_->GetMapChipIndexSetByPosition(posRightTopNew);

	bool hitLeft = false;
	bool hitRight = false;

	// --- 左上点の判定 ---
	MapChipField::MapChipType typeLeft = mapChipField_->GetMapChipTypeByIndex(indexSetLeftNew.xIndex, indexSetLeftNew.yIndex);
	if (typeLeft == MapChipField::MapChipType::kBlock && indexSetLeftNow.yIndex != indexSetLeftNew.yIndex) {
		hitLeft = true;
	}

	// --- 右上点の判定 ---
	MapChipField::MapChipType typeRight = mapChipField_->GetMapChipTypeByIndex(indexSetRightNew.xIndex, indexSetRightNew.yIndex);
	if (typeRight == MapChipField::MapChipType::kBlock && indexSetRightNow.yIndex != indexSetRightNew.yIndex) {
		hitRight = true;
	}

	// --- 衝突応答 ---
	if (hitLeft || hitRight) {
		float ceilingBottomY = FLT_MAX;
		if (hitLeft) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSetLeftNew.xIndex, indexSetLeftNew.yIndex);
			ceilingBottomY = (std::min)(ceilingBottomY, rect.bottom);
		}
		if (hitRight) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSetRightNew.xIndex, indexSetRightNew.yIndex);
			ceilingBottomY = (std::min)(ceilingBottomY, rect.bottom);
		}

		float playerTopY = worldTransform_.translation_.y + kHeight / 2.0f;
		info.moveVector.y = ceilingBottomY - playerTopY;
		info.ceilingCollision = true;
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
		DebugText::GetInstance()->ConsolePrintf("hit ceiling\n");
		velocity_.y = 0.0f; // 上昇中の速度をリセット
	}
}

void Player::GroundCollision(CollisionMapInfo& info) {
	// 下降中でなければ判定しない
	if (info.moveVector.y >= 0) {
		return;
	}

	// --- チェックポイントの準備 ---
	// 移動前の足元座標
	Vector3 posLeftBottomNow = CornerPosition(worldTransform_.translation_, kLeftBottom);
	Vector3 posRightBottomNow = CornerPosition(worldTransform_.translation_, kRightBottom);
	// 移動後の足元座標
	Vector3 posLeftBottomNew = CornerPosition(worldTransform_.translation_ + info.moveVector, kLeftBottom);
	Vector3 posRightBottomNew = CornerPosition(worldTransform_.translation_ + info.moveVector, kRightBottom);

	// 各座標のマップチップインデックスを取得
	MapChipField::IndexSet indexSetLeftNow = mapChipField_->GetMapChipIndexSetByPosition(posLeftBottomNow);
	MapChipField::IndexSet indexSetRightNow = mapChipField_->GetMapChipIndexSetByPosition(posRightBottomNow);
	MapChipField::IndexSet indexSetLeftNew = mapChipField_->GetMapChipIndexSetByPosition(posLeftBottomNew);
	MapChipField::IndexSet indexSetRightNew = mapChipField_->GetMapChipIndexSetByPosition(posRightBottomNew);

	bool hitLeft = false;
	bool hitRight = false;

	// --- 左下点の判定 ---
	// 移動後のチップが地面の表面か調べる
	MapChipField::MapChipType mapChipTypeLeft = mapChipField_->GetMapChipTypeByIndex(indexSetLeftNew.xIndex, indexSetLeftNew.yIndex);
	MapChipField::MapChipType mapChipTypeLeftNext = mapChipField_->GetMapChipTypeByIndex(indexSetLeftNew.xIndex, indexSetLeftNew.yIndex - 1);

	// 条件1: 移動先が地面の表面である
	bool isSurfaceLeft = (mapChipTypeLeft == MapChipField::MapChipType::kBlock && mapChipTypeLeftNext != MapChipField::MapChipType::kBlock);
	// 条件2: Y方向のマップチップインデックスが変わった（＝ブロックに侵入した瞬間）
	bool isJustEnteredLeft = (indexSetLeftNow.yIndex != indexSetLeftNew.yIndex);

	if (isSurfaceLeft && isJustEnteredLeft) {
		hitLeft = true;
	}

	// --- 右下点の判定 ---
	MapChipField::MapChipType mapChipTypeRight = mapChipField_->GetMapChipTypeByIndex(indexSetRightNew.xIndex, indexSetRightNew.yIndex);
	MapChipField::MapChipType mapChipTypeRightNext = mapChipField_->GetMapChipTypeByIndex(indexSetRightNew.xIndex, indexSetRightNew.yIndex - 1);

	bool isSurfaceRight = (mapChipTypeRight == MapChipField::MapChipType::kBlock && mapChipTypeRightNext != MapChipField::MapChipType::kBlock);
	bool isJustEnteredRight = (indexSetRightNow.yIndex != indexSetRightNew.yIndex);

	if (isSurfaceRight && isJustEnteredRight) {
		hitRight = true;
	}

	// --- 衝突応答 ---
	if (hitLeft || hitRight) {
		float playerBottomY = worldTransform_.translation_.y - kHeight / 2.0f;

		float groundTopY = -FLT_MAX;
		if (hitLeft) {
			MapChipField::Rect rectLeft = mapChipField_->GetRectByIndex(indexSetLeftNew.xIndex, indexSetLeftNew.yIndex);
			groundTopY = (std::max)(groundTopY, rectLeft.top);
		}
		if (hitRight) {
			MapChipField::Rect rectRight = mapChipField_->GetRectByIndex(indexSetRightNew.xIndex, indexSetRightNew.yIndex);
			groundTopY = (std::max)(groundTopY, rectRight.top);
		}

		info.moveVector.y = groundTopY - playerBottomY;
		info.groundCollision = true;
	}
}

void Player::ToggleOnGround(const CollisionMapInfo& info) {
	// 現在、空中状態の場合
	if (!onGround_) {
		// 地面との衝突が検出されたら、着地状態へ移行
		if (info.groundCollision) {
			onGround_ = true; // 着地状態に切り替える
			// 着地時にx速度を減衰
			velocity_.x *= (1.0f - kAttenuationLanding);
			// y速度をゼロにする（地面にめり込まないように）
			velocity_.y = 0.0f;
		}
	}
	// 現在、地上状態の場合
	else {
		// ジャンプなどでY速度が正になったら、空中状態へ移行
		if (velocity_.y > 0.0f) {
			onGround_ = false; // 空中状態に切り替える
			return;            // これ以降の地上チェックは不要
		}

		// 足元に地面があるかチェック（崖から落ちる場合など）
		const float kEpsilon = 0.1f; // 下方向への微小なチェック距離
		Vector3 posLeftBottom = CornerPosition(worldTransform_.translation_ + Vector3(0.0f, -kEpsilon, 0.0f), kLeftBottom);
		Vector3 posRightBottom = CornerPosition(worldTransform_.translation_ + Vector3(0.0f, -kEpsilon, 0.0f), kRightBottom);

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
	// 右方向への移動でなければ処理しない
	if (info.moveVector.x <= 0.0f) {
		return;
	}

	// --- チェックポイントの準備 ---
	Vector3 posRightTopNow = CornerPosition(worldTransform_.translation_, kRightTop);
	Vector3 posRightBottomNow = CornerPosition(worldTransform_.translation_, kRightBottom);
	Vector3 posRightTopNew = CornerPosition(worldTransform_.translation_ + info.moveVector, kRightTop);
	Vector3 posRightBottomNew = CornerPosition(worldTransform_.translation_ + info.moveVector, kRightBottom);

	MapChipField::IndexSet indexSetTopNow = mapChipField_->GetMapChipIndexSetByPosition(posRightTopNow);
	MapChipField::IndexSet indexSetBottomNow = mapChipField_->GetMapChipIndexSetByPosition(posRightBottomNow);
	MapChipField::IndexSet indexSetTopNew = mapChipField_->GetMapChipIndexSetByPosition(posRightTopNew);
	MapChipField::IndexSet indexSetBottomNew = mapChipField_->GetMapChipIndexSetByPosition(posRightBottomNew);

	bool hitTop = false;
	bool hitBottom = false;

	// --- 右上点の判定 ---
	MapChipField::MapChipType typeTop = mapChipField_->GetMapChipTypeByIndex(indexSetTopNew.xIndex, indexSetTopNew.yIndex);
	if (typeTop == MapChipField::MapChipType::kBlock && indexSetTopNow.xIndex != indexSetTopNew.xIndex) {
		hitTop = true;
	}

	// --- 右下点の判定 ---
	MapChipField::MapChipType typeBottom = mapChipField_->GetMapChipTypeByIndex(indexSetBottomNew.xIndex, indexSetBottomNew.yIndex);
	if (typeBottom == MapChipField::MapChipType::kBlock && indexSetBottomNow.xIndex != indexSetBottomNew.xIndex) {
		hitBottom = true;
	}

	// --- 衝突応答 ---
	if (hitTop || hitBottom) {
		float blockLeftX = FLT_MAX;
		if (hitTop) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSetTopNew.xIndex, indexSetTopNew.yIndex);
			blockLeftX = (std::min)(blockLeftX, rect.left);
		}
		if (hitBottom) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSetBottomNew.xIndex, indexSetBottomNew.yIndex);
			blockLeftX = (std::min)(blockLeftX, rect.left);
		}

		float playerRightX = worldTransform_.translation_.x + kWidth / 2.0f;
		info.moveVector.x = blockLeftX - playerRightX;
		info.wallCollision = true;
	}
}

void Player::LeftCollision(CollisionMapInfo& info) {
	// 左方向への移動でなければ処理しない
	if (info.moveVector.x >= 0.0f) {
		return;
	}

	// --- チェックポイントの準備 ---
	Vector3 posLeftTopNow = CornerPosition(worldTransform_.translation_, kLeftTop);
	Vector3 posLeftBottomNow = CornerPosition(worldTransform_.translation_, kLeftBottom);
	Vector3 posLeftTopNew = CornerPosition(worldTransform_.translation_ + info.moveVector, kLeftTop);
	Vector3 posLeftBottomNew = CornerPosition(worldTransform_.translation_ + info.moveVector, kLeftBottom);

	MapChipField::IndexSet indexSetTopNow = mapChipField_->GetMapChipIndexSetByPosition(posLeftTopNow);
	MapChipField::IndexSet indexSetBottomNow = mapChipField_->GetMapChipIndexSetByPosition(posLeftBottomNow);
	MapChipField::IndexSet indexSetTopNew = mapChipField_->GetMapChipIndexSetByPosition(posLeftTopNew);
	MapChipField::IndexSet indexSetBottomNew = mapChipField_->GetMapChipIndexSetByPosition(posLeftBottomNew);

	bool hitTop = false;
	bool hitBottom = false;

	// --- 左上点の判定 ---
	MapChipField::MapChipType typeTop = mapChipField_->GetMapChipTypeByIndex(indexSetTopNew.xIndex, indexSetTopNew.yIndex);
	if (typeTop == MapChipField::MapChipType::kBlock && indexSetTopNow.xIndex != indexSetTopNew.xIndex) {
		hitTop = true;
	}

	// --- 左下点の判定 ---
	MapChipField::MapChipType typeBottom = mapChipField_->GetMapChipTypeByIndex(indexSetBottomNew.xIndex, indexSetBottomNew.yIndex);
	if (typeBottom == MapChipField::MapChipType::kBlock && indexSetBottomNow.xIndex != indexSetBottomNew.xIndex) {
		hitBottom = true;
	}

	// --- 衝突応答 ---
	if (hitTop || hitBottom) {
		float blockRightX = -FLT_MAX;
		if (hitTop) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSetTopNew.xIndex, indexSetTopNew.yIndex);
			blockRightX = (std::max)(blockRightX, rect.right);
		}
		if (hitBottom) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSetBottomNew.xIndex, indexSetBottomNew.yIndex);
			blockRightX = (std::max)(blockRightX, rect.right);
		}

		float playerLeftX = worldTransform_.translation_.x - kWidth / 2.0f;
		info.moveVector.x = blockRightX - playerLeftX;
		info.wallCollision = true;
	}
}

void Player::OnWallCollision(const CollisionMapInfo& info) {

	//壁接触による減速
	if (info.wallCollision) {
		velocity_.x *= (1.0f - kAttenuationWall);
	}

}
