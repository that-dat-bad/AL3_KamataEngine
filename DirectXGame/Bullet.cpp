#include "Bullet.h"
#include "EnemyManager.h"
#include "MapChipField.h"

void Bullet::Initialize(Model* model, Camera* camera, const Vector3& position, const Vector3& velocity, EnemyManager* enemyManager, MapChipField* mapChipField) {
	assert(model);
	assert(camera);
	assert(enemyManager);

	model_ = model;
	camera_ = camera;
	velocity_ = velocity;
	enemyManager_ = enemyManager;
	mapChipField_ = mapChipField;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = kBulletSize;
}

void Bullet::Update() {
	lifeTimer_--;
	if (lifeTimer_ <= 0) {
		isDead_ = true;
	}

	worldTransform_.translation_ += velocity_;

	AABB myAABB = GetAABB();
	const std::list<Enemy*>& enemies = enemyManager_->GetEnemies();
#pragma region 敵との当たり判定
	for (Enemy* enemy : enemies) {
		if (enemy->IsDead()) {
			continue;
		}

		AABB enemyAABB = enemy->GetAABB();

		if ((myAABB.min.x <= enemyAABB.max.x && myAABB.max.x >= enemyAABB.min.x) && // X軸
		    (myAABB.min.y <= enemyAABB.max.y && myAABB.max.y >= enemyAABB.min.y) && // Y軸
		    (myAABB.min.z <= enemyAABB.max.z && myAABB.max.z >= enemyAABB.min.z)) { // Z軸

			enemy->OnCollision();
			isDead_ = true;
			break;
		}
	}
#pragma endregion

#pragma region 地形との当たり判定/反射

	Vector3 nextPosition = worldTransform_.translation_ + velocity_;

	MapChipField::IndexSet currentIndexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_);

	MapChipField::IndexSet xNextIndexSet = mapChipField_->GetMapChipIndexSetByPosition({nextPosition.x, worldTransform_.translation_.y, 0.0f});
	MapChipField::MapChipType xChipType = mapChipField_->GetMapChipTypeByIndex(xNextIndexSet.xIndex, xNextIndexSet.yIndex);
	if (xChipType == MapChipField::MapChipType::kBlock && currentIndexSet.xIndex != xNextIndexSet.xIndex) {
		if (currentBound_ < LimitBound) {
			velocity_.x *= -1.0f;
			currentBound_++;
		} else {
			isDead_ = true;
		}
	}

	MapChipField::IndexSet yNextIndexSet = mapChipField_->GetMapChipIndexSetByPosition({worldTransform_.translation_.x, nextPosition.y, 0.0f});
	MapChipField::MapChipType yChipType = mapChipField_->GetMapChipTypeByIndex(yNextIndexSet.xIndex, yNextIndexSet.yIndex);
	if (yChipType == MapChipField::MapChipType::kBlock && currentIndexSet.yIndex != yNextIndexSet.yIndex) {
		if (currentBound_ < LimitBound) {
			velocity_.y *= -1.0f;
			currentBound_++;
		} else {
			isDead_ = true;
		}
	}
#pragma endregion
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Bullet::Draw() { model_->Draw(worldTransform_, *camera_); }

AABB Bullet::GetAABB() {
	AABB aabb;
	// 弾の中心座標
	Vector3 center = worldTransform_.translation_;
	// 弾のサイズ
	Vector3 halfSize = {kBulletSize.x * 0.5f, kBulletSize.y * 0.5f, kBulletSize.z * 0.5f};
	aabb.min = {center.x - halfSize.x, center.y - halfSize.y, center.z - halfSize.z};
	aabb.max = {center.x + halfSize.x, center.y + halfSize.y, center.z + halfSize.z};
	return aabb;
}
