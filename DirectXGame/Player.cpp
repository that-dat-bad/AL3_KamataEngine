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
	//1.移動入力
	if (onGround_) {
		// 接地状態の処理
		// ジャンプ入力で空中状態へ移行
		if (Input::GetInstance()->PushKey(DIK_UP)) {
			velocity_.y += kJumpAcceleration;
			onGround_ = false;
		}

		// 左右移動入力
		if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {

			// 左右加速
			Vector3 acceleration = {};
			if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
				// 逆方向に入力されたらブレーキをかける
				if (velocity_.x < 0.0f) {
					velocity_.x *= (1.0f - kAttenuation);
				}
				acceleration.x += kAcceleration;
				// 右向きに変更された瞬間の処理
				if (lrdirection_ != LRDirection::kRight) {
					lrdirection_ = LRDirection::kRight;
					// 旋回開始時の角度を記録
					turnFIrstRotationY_ = worldTransform_.rotation_.y;
					// 旋回タイマーに時間を設定する
					turnTimer_ = kTimeTurn;
				}
			} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
				// 逆方向に入力されたらブレーキをかける
				if (velocity_.x > 0.0f) {
					velocity_.x *= (1.0f - kAttenuation);
				}
				acceleration.x -= kAcceleration;
				// 左向きに変更された瞬間の処理
				if (lrdirection_ != LRDirection::kLeft) {
					lrdirection_ = LRDirection::kLeft;
					// 旋回開始時の角度を記録
					turnFIrstRotationY_ = worldTransform_.rotation_.y;
					// 旋回タイマーに時間を設定する
					turnTimer_ = kTimeTurn;
				}
			}

			// 速度に加速度を加算
			velocity_.x += acceleration.x;
			velocity_.y += acceleration.y;
			velocity_.z += acceleration.z;
			// 最大速度制限
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
		} else {
			// 移動入力がない場合は摩擦で横方向の速度を減衰させる
			velocity_.x *= (1.0f - kAttenuation);
		}

	} else {
		// 空中状態の処理（重力のみ）
		velocity_.y -= kGravityAcceleration;
		velocity_.y = (std::max)(velocity_.y, -kLimitFallSpeed);
	}







	//衝突情報を初期化
	CollisionMapInfo collisionMapInfo;
	//移動量に速度の値をコピー
	collisionMapInfo.moveVector = velocity_;
	//マップ衝突チェック
	MapCollider(collisionMapInfo);

	ResultReflectToMove(collisionMapInfo); // 衝突結果を座標に反映
	OnCeilingCollision(collisionMapInfo);  // 衝突後の処理（速度リセットなど）

	// 地面との当たり判定（空中かつ落下中のみ）
	if (!onGround_ && velocity_.y < 0) {
		// Y座標が地面以下になったら着地として補正
		if (worldTransform_.translation_.y <= 2.0f) {
			// めり込み排斥
			worldTransform_.translation_.y = 2.0f;
			// 摩擦で横方向速度が減衰
			velocity_.x *= (1.0f - kAttenuation);
			// 下方向速度をリセット
			velocity_.y = 0.0f;
			// 接地状態に移行
			onGround_ = true;
		}
	}

	// 旋回制御
	if (turnTimer_ > 0.0f) {
		// 旋回タイマーを減少
		turnTimer_ -= 1.0f / 60.0f;

		// 左右の目標角度テーブル
		float destinationRotationYTable[] = {
		    std::numbers::pi_v<float> / 2.0f,
		    std::numbers::pi_v<float> * 3.0f / 2.0f,
		};

		// 状態に応じた目標角度を取得する
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrdirection_)];

		// 旋回の補間計算
		float progress = 1.0f - (turnTimer_ / kTimeTurn); // 0.0f ~ 1.0fの進捗度
		worldTransform_.rotation_.y = turnFIrstRotationY_ + (destinationRotationY - turnFIrstRotationY_) * progress;
	}

	// 行列更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	// 行列を定数バッファに転送
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
	//FloorCollision(info);
}

void Player::CeilingCollision(CollisionMapInfo& info) {
	// 上昇中でなければ処理しない
	if (info.moveVector.y <= 0) {
		return;
	}


	Vector3 posLeftTopNew = CornerPosition(worldTransform_.translation_ + info.moveVector, kLeftTop);
	Vector3 posRightTopNew = CornerPosition(worldTransform_.translation_ + info.moveVector, kRightTop);

	MapChipField::MapChipType mapChipType;
	bool hit = false;
	MapChipField::IndexSet indexSet;

	// 左上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(posLeftTopNew);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipField::MapChipType::kBlock) {
		hit = true;
	}

	// 右上点の判定
	if (!hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(posRightTopNew);
		mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
		if (mapChipType == MapChipField::MapChipType::kBlock) {
			hit = true;
		}
	}

	if (hit) {
	//めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(posLeftTopNew);
	
		//めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.moveVector.y = rect.bottom - worldTransform_.translation_.y - (kHeight / 2.0f);
		//天井にあたったことを記録する
		info.ceilingCollision = true;
	}
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) { 
	Vector3 offsetTable[kNumCorner] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0.0f},
        {-kWidth / 2.0f, -kHeight / 2.0f, 0.0f},
        {-kWidth / 2.0f, +kHeight / 2.0f, 0.0f},
        {+kWidth / 2.0f, +kHeight / 2.0f, 0.0f}
    };
	// 中心座標にオフセットを加算してコーナー座標を計算
	return center + offsetTable[static_cast<uint32_t>(corner)];
}

void Player::ResultReflectToMove(const CollisionMapInfo& info) {
//移動
	worldTransform_.translation_ += info.moveVector;
}

void Player::OnCeilingCollision(const CollisionMapInfo& info) {
	if (info.ceilingCollision) {
		DebugText::GetInstance()->ConsolePrintf("hit ceiling\n");
		velocity_.y = 0.0f; // 上昇中の速度をリセット
	}
}
