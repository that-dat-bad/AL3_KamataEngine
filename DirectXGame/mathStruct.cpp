#include "mathStruct.h"



Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 buf;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {

			buf.m[i][j] = m1.m[i][0] * m2.m[0][j] + m1.m[i][1] * m2.m[1][j] + m1.m[i][2] * m2.m[2][j] + m1.m[i][3] * m2.m[3][j];
		}
	}
	return buf;
}

Matrix4x4 Identity4x4() {
	Matrix4x4 buf;
	buf = {0};
	buf.m[0][0] = 1;
	buf.m[1][1] = 1;
	buf.m[2][2] = 1;
	buf.m[3][3] = 1;

	return buf;
}

Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {

	Matrix4x4 buf;
	buf = Identity4x4();
	buf.m[3][0] = translate.x;
	buf.m[3][1] = translate.y;
	buf.m[3][2] = translate.z;
	return buf;
}

Matrix4x4 MakeScaleMatrix(const Vector3& scale) {

	Matrix4x4 buf;
	buf = Identity4x4();
	buf.m[0][0] = scale.x;
	buf.m[1][1] = scale.y;
	buf.m[2][2] = scale.z;
	buf.m[3][3] = 1;
	return buf;
}

Matrix4x4 MakeRotateXMatrix(float radian) {
	Matrix4x4 buf;
	buf = Identity4x4();
	buf.m[1][1] = std::cos(radian);
	buf.m[2][1] = -std::sin(radian);
	buf.m[1][2] = std::sin(radian);
	buf.m[2][2] = std::cos(radian);
	return buf;
}

Matrix4x4 MakeRotateYMatrix(float radian) {
	Matrix4x4 buf;
	buf = Identity4x4();
	buf.m[0][0] = std::cos(radian);
	buf.m[2][0] = std::sin(radian);
	buf.m[0][2] = -std::sin(radian);
	buf.m[2][2] = std::cos(radian);
	return buf;
}

Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 buf;
	buf = Identity4x4();
	buf.m[0][0] = std::cos(radian);
	buf.m[0][1] = std::sin(radian);
	buf.m[1][0] = -std::sin(radian);
	buf.m[1][1] = std::cos(radian);
	return buf;
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	// 拡大縮小行列
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);

	// 回転行列の生成
	Matrix4x4 rotateX = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateY = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZ = MakeRotateZMatrix(rotate.z);

	// 平行移動行列
	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);

	return Multiply(Multiply(Multiply(Multiply(scaleMatrix, rotateX), rotateY), rotateZ), translateMatrix);
}

Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t) {
	Vector3 buf;
	buf.x = v1.x + (v2.x - v1.x) * t;
	buf.y = v1.y + (v2.y - v1.y) * t;
	buf.z = v1.z + (v2.z - v1.z) * t;
	return buf;
}

Vector3 operator+(const Vector3& v1, const Vector3& v2) {
	Vector3 buf;
	buf.x = v1.x + v2.x;
	buf.y = v1.y + v2.y;
	buf.z = v1.z + v2.z;
	return buf;	
}

Vector3 operator+=(Vector3& v1, const Vector3& v2) { 
	v1.x += v2.x;
	v1.y += v2.y;
	v1.z += v2.z;
	return v1;
}

bool AABBCollision(const AABB& aabb1, const AABB& aabb2) {
	if ((aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x) && (aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y) && (aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z)) {
		return true;
	} else {
		return false;
	}
}

Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result;
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	result.z /= w;
	return result;
}


float EaseIn(float start, float end, float t) {
	float result = end - start;
	return start + result * t * t;
}

float EaseOut(float start, float end, float t) {
	float result = end - start;
	return start + result * (1.0f - (1.0f - t) * (1.0f - t));
}
