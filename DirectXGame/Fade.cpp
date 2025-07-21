#include "Fade.h"
using namespace KamataEngine;
void Fade::Initialize() {
	// スプライトを生成
	// 1x1の白テクスチャを読み込む
	uint32_t textureHandle = TextureManager::Load("white1x1.png");
	// 読み込んだテクスチャを元にスプライトを生成
	sprite_ = Sprite::Create(textureHandle, {0.0f, 0.0f});
	// 画面全体を覆うサイズに設定
	sprite_->SetSize({1280, 720});

	// 色を黒・不透明に設定 (RGBA)
	sprite_->SetColor({0, 0, 0, 1});
}

void Fade::Update() {
	// 状態に応じて処理を分岐
	switch (status_) {
	case Status::FadeIn:
		// フェードインの処理
		counter_ += 1.0f / 60.0f;
		counter_ = (std::min)(counter_, duration_); // カウンタが時間を超えないように
		// 徐々に透明にする (アルファ値を 1 -> 0 へ)
		sprite_->SetColor({0, 0, 0, 1.0f - (counter_ / duration_)});
		break;

	case Status::FadeOut:
		// フェードアウトの処理
		counter_ += 1.0f / 60.0f;
		counter_ = (std::min)(counter_, duration_); // カウンタが時間を超えないように
		// 徐々に不透明にする (アルファ値を 0 -> 1 へ)
		sprite_->SetColor({0, 0, 0, counter_ / duration_});
		break;

	case Status::None:
		// 何もしない
		break;
	}
}

void Fade::Draw() {
	// フェード中でなければ描画しない
	if (status_ == Status::None) {
		return;
	}

	// スプライトの描画は、必ずPreDrawとPostDrawの間で呼ぶ
	Sprite::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	sprite_->Draw();
	Sprite::PostDraw();
}

void Fade::Start(Status status, float duration) {
	status_ = status;
	duration_ = duration;
	counter_ = 0.0f; // カウンターをリセット
}

void Fade::Stop() { status_ = Status::None; }

bool Fade::IsFinished() const {
	// FadeInかFadeOutの最中で、カウンターが時間に達していたら終了
	if (status_ == Status::FadeIn || status_ == Status::FadeOut) {
		return counter_ >= duration_;
	}
	return true; // それ以外の状態は即時終了とみなす
}