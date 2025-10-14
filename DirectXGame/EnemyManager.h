#pragma once
#include "Enemy.h"
#include <list>

class EnemyManager {
public:
	~EnemyManager();

	// 初期化
	void Initialize(Model* model, Camera* camera);
	// 更新
	void Update();
	// 描画
	void Draw();

	// 敵の生成
	void SpawnEnemy(const Vector3& position);

	const std::list<Enemy*>& GetEnemies() const { return enemies_; }

private:
	Model* model_ = nullptr;
	Camera* camera_ = nullptr;
	std::list<Enemy*> enemies_;
};