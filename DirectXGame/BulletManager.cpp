#include "BulletManager.h"

BulletManager::~BulletManager() {
	for (Bullet* bullet : bullets_) {
		delete bullet;
	}
	bullets_.clear();
}

void BulletManager::Initialize(Model* model, Camera* camera, EnemyManager* enemyManager) {
	model_ = model;
	camera_ = camera;
	enemyManager_ = enemyManager;
}

void BulletManager::Update() {

	for (Bullet* bullet : bullets_) {
		bullet->Update();
	}

	bullets_.remove_if([](Bullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});
}

void BulletManager::Draw() {
	for (Bullet* bullet : bullets_) {
		bullet->Draw();
	}
}

void BulletManager::SpawnBullet(const Vector3& position, const Vector3& velocity) {

	Bullet* newBullet = new Bullet();
	newBullet->Initialize(model_, camera_, position, velocity,enemyManager_);
	bullets_.push_back(newBullet);
}