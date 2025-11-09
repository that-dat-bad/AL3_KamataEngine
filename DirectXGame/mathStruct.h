#pragma once
#include "KamataEngine.h"
using namespace KamataEngine;

struct AABB {
	Vector3 min;
	Vector3 max;
};

Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

Matrix4x4 Identity4x4();

Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

Matrix4x4 MakeScaleMatrix(const Vector3& scale);

Matrix4x4 MakeRotateXMatrix(float radian);

Matrix4x4 MakeRotateYMatrix(float radian);

Matrix4x4 MakeRotateZMatrix(float radian);

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

/// <summary>
/// 行列を計算・転送する
/// </summary>
void UpdateWorldMatrix(KamataEngine::WorldTransform& worldTransform);

Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);

Vector3 operator+(const Vector3& v1, const Vector3& v2);

Vector3 operator+=(Vector3& v1, const Vector3& v2);

bool AABBCollision(const AABB& aabb1, const AABB& aabb2);

Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);

// ベクトル変換
Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m);

// EaseIn
float EaseIn(float start, float end, float t);

// EaseOut
float EaseOut(float start, float end, float t);

Vector3 operator*(const Vector3& v, float s);
Vector3 operator/(const Vector3& v, float s);
Vector3& operator*=(Vector3& v, float s);
Vector3& operator/=(Vector3& v, float s);