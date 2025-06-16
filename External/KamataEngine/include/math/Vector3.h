#pragma once

namespace KamataEngine {

/// <summary>
/// 3次元ベクトル
/// </summary>
struct Vector3 final {
	float x;
	float y;
	float z;

	// 加算演算子のオーバーロード
	Vector3 operator+(const Vector3& other) const { return {x + other.x, y + other.y, z + other.z}; }
};

} // namespace KamataEngine