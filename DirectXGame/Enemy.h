#pragma once
#include "EnemyBullet.h"
#include "KamataEngine.h"
#include <list>
#include <stdint.h>

/// <summary>
/// 敵
/// </summary>
class Enemy {
public:
	~Enemy();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="position">初期座標</param>
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position);

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
	/// <returns></returns>
	bool IsDead() const { return isDead_; }

	void SetBulletModel(KamataEngine::Model* model) { bulletModel_ = model; };

private:
	/// <summary>
	/// 接近フェーズの更新
	/// </summary>
	void UpdateApproach();

	/// <summary>
	/// 離脱フェーズの更新
	/// </summary>
	void UpdateLeave();

	/// <summary>
	/// 弾発射
	/// </summary>
	void Fire();

	// メンバ関数ポインタ型を宣言
	using StateFunction = void (Enemy::*)();
	// 現在のステートを保持するメンバ関数ポインタ
	StateFunction stateFunction_ = nullptr;

	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;
	// モデル
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* bulletModel_ = nullptr;

	// デスフラグ
	bool isDead_ = false;

	// 敵の弾リスト
	std::list<EnemyBullet*> bullets_;
};