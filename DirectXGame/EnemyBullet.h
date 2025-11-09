#pragma once
#include "KamataEngine.h"
#include <stdint.h>

/// <summary>
/// 敵の弾
/// </summary>
class EnemyBullet {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="position">初期座標</param>
	/// <param name="velocity">速度</param>
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="camera">カメラ</param>
	void Draw(const KamataEngine::Camera& camera);

	/// <summary>
	/// 死亡フラグを取得
	/// </summary>
	bool IsDead() const { return isDead_; }

private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;
	// モデルのポインタ
	KamataEngine::Model* model_ = nullptr;
	// テクスチャハンドル
	uint32_t textureHandle_ = 0;

	// 速度
	KamataEngine::Vector3 velocity_;

	// 寿命<frm> (PlayerBulletと同様)
	static const int32_t kLifeTime = 60 * 5;
	// デスタイマー
	int32_t deathTimer_ = kLifeTime;
	// デスフラグ
	bool isDead_ = false;
};