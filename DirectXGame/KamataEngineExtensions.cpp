#include "KamataEngineExtensions.h"

// 静的メンバ変数の実体を定義
std::map<Sprite*, Vector2> KamataEngineExtensions::originalSpriteSizes_;
std::map<uint32_t, Sprite*> KamataEngineExtensions::utilitySprites_;

void KamataEngineExtensions::Finalize() {
	// utilitySprites_ に保存された全てのスプライトを解放する
	for (auto& pair : utilitySprites_) {
		delete pair.second;
	}
	utilitySprites_.clear();
}

void KamataEngineExtensions::SetSpriteScale(Sprite* sprite, float scale) {
	// このスプライトの元のサイズがまだ保存されていなければ、保存する
	if (originalSpriteSizes_.find(sprite) == originalSpriteSizes_.end()) {
		originalSpriteSizes_[sprite] = sprite->GetSize();
	}

	// 元のサイズを取得
	Vector2 originalSize = originalSpriteSizes_[sprite];

	// 元のサイズに拡大率を掛けて、新しいサイズを計算
	Vector2 newSize = {originalSize.x * scale, originalSize.y * scale};

	// KamataEngineが元々持っているSetSize関数を使って、サイズを変更する
	sprite->SetSize(newSize);
}

void KamataEngineExtensions::DrawSpriteFixed(uint32_t textureHandle, const Vector2& position, const Vector2& size, const Vector4& color) {
	// このテクスチャ用のスプライトがまだ作られていなければ、新しく作る
	if (utilitySprites_.find(textureHandle) == utilitySprites_.end()) {
		utilitySprites_[textureHandle] = Sprite::Create(textureHandle, {0, 0});
	}

	// 対応するスプライトを取得
	Sprite* sprite = utilitySprites_[textureHandle];

	// 指定されたパラメータを設定
	sprite->SetPosition(position);
	sprite->SetSize(size);
	sprite->SetColor(color);

	// 描画
	sprite->Draw();
}