#pragma once
#include "KamataEngine.h"  // (スライド 154319.png)
#include <stdint.h>         // (textureHandle_ のため)

/// <summary>
/// 自キャラの弾 (スライド 154319.png)
/// </summary>
class PlayerBullet {
public: // (スライド 154324.png)
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
	/// 描画 (スライド 154331.png)
	/// </summary>
	/// <param name="camera">カメラ</param>
	void Draw(const KamataEngine::Camera& camera);

private: // (スライド 154335.png)
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;
	// モデルのポインタ（借りてくるやつ）
	KamataEngine::Model* model_ = nullptr;
	// テクスチャハンドル
	uint32_t textureHandle_ = 0;
};