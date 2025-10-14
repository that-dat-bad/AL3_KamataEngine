#include "Bullet.h"
#include"EnemyManager.h"

void Bullet::Initialize(Model* model, Camera* camera, const Vector3& position, const Vector3& velocity, EnemyManager* enemyManager)
{
	assert(model);
	assert(camera);
	assert(enemyManager);

	model_ = model;
	camera_ = camera;
	velocity_ = velocity;
	enemyManager_ = enemyManager;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = kBulletSize;
}

void Bullet::Update() {
	// 寿命が尽きたらフラグを立てる
	lifeTimer_--;
	if (lifeTimer_ <= 0) {
		isDead_ = true;
	}

	worldTransform_.translation_ += velocity_;


	AABB myAABB = GetAABB();
	const std::list<Enemy*>& enemies = enemyManager_->GetEnemies();

	for (Enemy* enemy : enemies) {
		if (enemy->IsDead()) {
			continue;
		}

		AABB enemyAABB = enemy->GetAABB();

		// X軸、Y軸、Z軸すべてで重なりがあるかチェック
		if ((myAABB.min.x <= enemyAABB.max.x && myAABB.max.x >= enemyAABB.min.x) && // X軸
		    (myAABB.min.y <= enemyAABB.max.y && myAABB.max.y >= enemyAABB.min.y) && // Y軸
		    (myAABB.min.z <= enemyAABB.max.z && myAABB.max.z >= enemyAABB.min.z)) { // Z軸

			// 衝突
			enemy->OnCollision();
			isDead_ = true;
			break; // この弾は消えるのでループを抜ける
		}
	}
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Bullet::Draw() {
	model_->Draw(worldTransform_, *camera_); }

AABB Bullet::GetAABB() {
    AABB aabb;
    // 弾の中心座標
    Vector3 center = worldTransform_.translation_;
    // 弾のサイズ
    Vector3 halfSize = { kBulletSize.x * 0.5f, kBulletSize.y * 0.5f, kBulletSize.z * 0.5f };
    aabb.min = { center.x - halfSize.x, center.y - halfSize.y, center.z - halfSize.z };
    aabb.max = { center.x + halfSize.x, center.y + halfSize.y, center.z + halfSize.z };
    return aabb;
}
