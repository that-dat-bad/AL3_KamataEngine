#include "GameScene.h"
using namespace KamataEngine;

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

// デストラクタ
GameScene::~GameScene() {
	delete model_;
	delete player_;
	delete blockModel_;
	delete skydomeModel_;
	delete skydome_;
	for (auto& row : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : row) {
			delete worldTransformBlock; // 各ワールド変換を解放
		}
		row.clear(); // 行をクリア
	}
	worldTransformBlocks_.clear();
	delete debugCamera_; // デバッグカメラの解放
	delete mapChipField_;
}

void GameScene::Initialize() {
	// ファイル名を指定してテクスチャを読み込む
	textureHandle_ = TextureManager::Load("UVchecker.png");

	// 3Dモデルの生成
	model_ = Model::Create();
	blockModel_ = Model::Create();
	skydomeModel_ = Model::CreateFromOBJ("ball", true);

	// カメラの初期化
	camera_.Initialize();

	// 自キャラ生成
	player_ = new Player();
	// 自キャラの初期化
	player_->Initialize(model_, textureHandle_, &camera_);

	// 天球の生成
	skydome_ = new Skydome();

	// 天球の初期化
	skydome_->Initialize(skydomeModel_, textureHandle_, &camera_);

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);

	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");
	GenerateBlocks();
}

void GameScene::Update() {
	// 自キャラの更新
	player_->Update();

	// ブロックの更新
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}

			// アフィン変換の作成
			Matrix4x4 affineMatrix = MakeAffineMatrix(worldTransformBlock->scale_, worldTransformBlock->rotation_, worldTransformBlock->translation_);
			worldTransformBlock->matWorld_ = affineMatrix;
			worldTransformBlock->TransferMatrix();
		}
	}
	// デバック時のみキーを押したときデバックカメラを有効化
#ifdef _DEBUG

	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}

#endif // DEBUG

	if (isDebugCameraActive_) {
		// デバッグカメラの更新
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;

		// ビュープロジェクション行列の転送
		camera_.TransferMatrix();
	} else {
		camera_.UpdateMatrix();
	}
}

void GameScene::Draw() {

	// 自キャラの描画
	player_->Draw();
	skydome_->Draw();

	Model::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	// ブロックの描画
	for (uint32_t i = 0; i < worldTransformBlocks_.size(); ++i) {
		for (uint32_t j = 0; j < worldTransformBlocks_[i].size(); ++j) {
			WorldTransform* worldTransformBlock = worldTransformBlocks_[i][j];
			if (!worldTransformBlock) {
				continue;
			}

			blockModel_->Draw(*worldTransformBlock, camera_);
		}
	}
	Model::PostDraw();
}

void GameScene::GenerateBlocks() {
	const float kBlockWidth = 2.0f;
	const float kBlockHeight = 2.0f;
	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	worldTransformBlocks_.resize(numBlockVirtical);
	for (uint32_t i = 0; i < numBlockVirtical; i++) {
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}

	// ブロックの生成
	for (uint32_t i = 0; i < numBlockVirtical; i++) {
		for (uint32_t j = 0; j < numBlockHorizontal; j++) {
			// MapChipField からマップチップのタイプを取得
			MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(j, i);

			if (mapChipType != MapChipType::kBlank) {
				worldTransformBlocks_[i][j] = new WorldTransform();
				worldTransformBlocks_[i][j]->Initialize();
				worldTransformBlocks_[i][j]->translation_.x = kBlockWidth * j;
				worldTransformBlocks_[i][j]->translation_.y = kBlockHeight * i;
			} else {
				// ブロックがない場合は nullptr を設定
				worldTransformBlocks_[i][j] = nullptr;
			}
		}
	}
}
