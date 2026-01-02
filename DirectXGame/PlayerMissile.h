#pragma once
#include "KamataEngine.h"

// 前方宣言（Enemyクラスを使うため）
class Enemy;

class PlayerMissile {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">ミサイルのモデル</param>
	/// <param name="position">発射位置</param>
	/// <param name="target">狙う敵</param>
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position, Enemy* target);

	void Update();
	void Draw(const KamataEngine::Camera& camera);

	bool IsDead() const { return isDead_; }
	void OnCollision();
	KamataEngine::Vector3 GetWorldPosition() const { return worldTransform_.translation_; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	uint32_t textureHandle_ = 0;

	// 誘導対象の敵
	Enemy* target_ = nullptr;

	// 速度
	KamataEngine::Vector3 velocity_;
	// 速度の大きさ
	const float kMissileSpeed_ = 1.5f;
	// 誘導性能（0.0～1.0）
	const float kHomingStrength_ = 0.08f;

	// 寿命
	int32_t deathTimer_ = 300; // 5秒
	bool isDead_ = false;
};