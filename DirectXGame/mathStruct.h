#pragma once
#include "KamataEngine.h"
#include "math\Vector3.h"
#include <cmath>
using namespace KamataEngine;

Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

Matrix4x4 Identity4x4();

Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

Matrix4x4 MakeScaleMatrix(const Vector3& scale);

Matrix4x4 MakeRotateXMatrix(float radian);

Matrix4x4 MakeRotateYMatrix(float radian);

Matrix4x4 MakeRotateZMatrix(float radian);

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);

Vector3 operator+(const Vector3& v1, const Vector3& v2);

Vector3 operator+=(Vector3& v1, const Vector3& v2);



inline KamataEngine::Vector3 operator*(const KamataEngine::Vector3& v, float s) {
    return { v.x * s, v.y * s, v.z * s };
}

inline KamataEngine::Vector3 operator*(float s, const KamataEngine::Vector3& v) {
    return v * s;
}

inline KamataEngine::Vector3 operator-(const KamataEngine::Vector3& lhs, const KamataEngine::Vector3& rhs) {
    return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
}

inline KamataEngine::Vector3 operator+(const KamataEngine::Vector3& lhs, const KamataEngine::Vector3& rhs) {
    return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
}

inline float Length(const KamataEngine::Vector3& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}