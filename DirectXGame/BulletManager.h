#pragma once
#include "Bullet.h"
#include <list>
#include"EnemyManager.h"

class EnemyManager;

class BulletManager {
public:
	~BulletManager();

	// 初期化
	void Initialize(Model* model, Camera* camera,EnemyManager* enemyManager);
	// 更新
	void Update();
	// 描画
	void Draw();

	// 弾の生成
	void SpawnBullet(const Vector3& position, const Vector3& velocity);

private:
	// 弾のモデルとカメラ
	Model* model_ = nullptr;
	Camera* camera_ = nullptr;
	// 弾のリスト
	std::list<Bullet*> bullets_;

	EnemyManager* enemyManager_ = nullptr;
};