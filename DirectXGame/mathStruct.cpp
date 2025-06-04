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