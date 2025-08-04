// HitEffect.h
#pragma once
#include "KamataEngine.h"
#include <array>
#include <random>

using namespace KamataEngine;

class HitEffect {
public:
	// --- アニメーションの状態 ---
	enum class State {
		kSpread, // 広がる
		kFade,   // 消える
		kDead,   // 死亡
	};

	// --- 静的メンバ関数 ---
	static HitEffect* Create(const Vector3& position, const Vector3& rotation);
	static void SetModel(Model* model) { model_ = model; }
	static void SetCamera(Camera* camera) { camera_ = camera; }

	// --- 通常のメンバ関数 ---
	void Initialize(const Vector3& position, const Vector3& rotation);
	void Update();
	void Draw();
	// デスフラグの取得
	bool IsDead() const { return state_ == State::kDead; }

private:
	// --- 静的メンバ変数 ---
	static Model* model_;
	static Camera* camera_;

	// --- 定数 ---
	static inline const uint32_t kNumStreaks = 2;      // 筋の個数
	static inline const uint32_t kSpreadDuration = 30; // 広がる時間
	static inline const uint32_t kFadeDuration = 60;   // 消える時間

	// --- メンバ変数 ---
	State state_ = State::kSpread;
	uint32_t counter_ = 0;
	ObjectColor objectColor_; // 色変更用

	WorldTransform circleWorldTransform_;                             // 円のトランスフォーム
	std::array<WorldTransform, kNumStreaks> streakWorldTransforms_;

	// 乱数生成用
	std::mt19937_64 randomEngine_;
};