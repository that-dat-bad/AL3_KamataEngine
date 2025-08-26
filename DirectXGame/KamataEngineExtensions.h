#pragma once
#include "KamataEngine.h"
#include <map>

using namespace KamataEngine;

// KamataEngineの機能を拡張するための静的クラス
class KamataEngineExtensions {
public:
	// ゲーム終了時に呼ぶ後片付け用の関数
	static void Finalize();

	// スプライトに拡大縮小機能を設定する
	static void SetSpriteScale(Sprite* sprite, float scale);

	// 指定したテクスチャを、指定した場所・大きさで描画する
	static void DrawSpriteFixed(uint32_t textureHandle, const Vector2& position, const Vector2& size, const Vector4& color = {1.0f, 1.0f, 1.0f, 1.0f});

private:
	// 各スプライトの元のサイズを保存するためのマップ
	static std::map<Sprite*, Vector2> originalSpriteSizes_;
	// DrawSpriteFixedで使うための、テクスチャごとのスプライトを保存するマップ
	static std::map<uint32_t, Sprite*> utilitySprites_;
};