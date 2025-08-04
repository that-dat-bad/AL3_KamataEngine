#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

class HitEffect {
public:
	// --- 静的メンバ関数 ---
	static HitEffect* Create(const Vector3& position, const Vector3& rotation);
	static void SetModel(Model* model) { model_ = model; }
	static void SetCamera(Camera* camera) { camera_ = camera; }

	// --- 通常のメンバ関数 ---
	void Initialize(const Vector3& position, const Vector3& rotation);
	void Update();
	void Draw();

private:
	// --- 静的メンバ変数 (全インスタンスで共有) ---
	static Model* model_;
	static Camera* camera_;

	// --- 通常のメンバ変数 (インスタンスごと) ---
	WorldTransform circleWorldTransform_;
};