#include "DeathParticles.h"
#include "mathStruct.h"
#include <algorithm> 

void DeathParticles::Initialize(Model* model, Camera* camera, const Vector3& position) {
	// 引数をメンバ変数に記録
	model_ = model;
	camera_ = camera;

	// 色変更オブジェクトの初期化
	objectColor_.Initialize();
	color_ = {1, 1, 1, 1}; // 初期色は白

	// 全てのパーティクルのワールドトランスフォームを初期化
	for (WorldTransform& worldTransform : worldTransforms_) {
		worldTransform.Initialize();
		worldTransform.translation_ = position; // 全て同じ位置で初期化
	}
}

void DeathParticles::Update() {
	// 終了していたら何もしない
	if (isFinished_) {
		return;
	}

	// タイマーと終了フラグの更新
	counter_ += 1.0f / 60.0f;
	if (counter_ >= kDuration) {
		counter_ = kDuration;
		isFinished_ = true;
	}

	// フェードアウト処理
	float alpha = 1.0f - (counter_ / kDuration);
	color_.w = std::clamp(alpha, 0.0f, 1.0f);
	objectColor_.SetColor(color_);

	// 全てのパーティクルを動かす
	for (uint32_t i = 0; i < kNumParticles; ++i) {
		// 基本となる速度ベクトル
		Vector3 velocity = {kSpeed, 0, 0};
		// 回転角を計算
		float angle = kAngleUnit * i;
		// Z軸まわり回転行列
		Matrix4x4 matrixRotation = MakeRotateZMatrix(angle);
		// 基本ベクトルを回転させて速度ベクトルを得る
		velocity = Transform(velocity, matrixRotation);

		// 移動処理
		worldTransforms_[i].translation_ += velocity;

		// 行列の更新と転送
		worldTransforms_[i].matWorld_ = MakeAffineMatrix(worldTransforms_[i].scale_, worldTransforms_[i].rotation_, worldTransforms_[i].translation_);
		worldTransforms_[i].TransferMatrix();
	}
}

void DeathParticles::Draw() {
	// 終了していたら何もしない
	if (isFinished_) {
		return;
	}

	// 全てのパーティクルを描画
	Model::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	for (WorldTransform& worldTransform : worldTransforms_) {
		// 色変更オブジェクトを渡して描画
		model_->Draw(worldTransform, *camera_, &objectColor_);
	}
	Model::PostDraw();
}