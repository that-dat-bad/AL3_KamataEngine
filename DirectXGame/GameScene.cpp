#include "GameScene.h"
using namespace KamataEngine;
#include "mathStruct.h"
#include <random>

// デストラクタ
GameScene::~GameScene() {
	// delete model_; // 初期化されていないため削除
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
	delete cameraController_; // カメラコントローラの解放

	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	enemies_.clear(); // vectorをクリア
	delete deathParticles_;
	delete deathParticleModel_;
}

void GameScene::Initialize() {
	// ファイル名を指定してテクスチャを読み込む
	textureHandle_ = TextureManager::Load("UVchecker.png");

	// 3Dモデルの生成
	playerModel_ = Model::CreateFromOBJ("player", true);
	blockModel_ = Model::Create();
	skydomeModel_ = Model::CreateFromOBJ("ball", true);
	enemyModel_ = Model::CreateFromOBJ("enemy", true);
	deathParticleModel_ = Model::CreateFromOBJ("deathParticle", true);
	// カメラの初期化
	camera_.Initialize();

	// マップチップフィールドの生成と初期化
	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");
	GenerateBlocks();

	// 自キャラ生成
	player_ = new Player();
	// 座標をマップチップ番号で指定
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(0, 18);
	// 自キャラの初期化
	player_->Initialize(playerModel_, &camera_, playerPosition);

	player_->SetMapChipField(mapChipField_);

	// --- デスパーティクルの生成テスト ---
	deathParticles_ = new DeathParticles();
	// プレイヤーと同じ位置に生成
	deathParticles_->Initialize(deathParticleModel_, &camera_, player_->GetWorldPosition());

	// 敵キャラ生成
	kEnemyCount_ = 2; // 敵キャラの数を定義

	for (int32_t i = 0; i < kEnemyCount_; i++) {
		Enemy* newEnemy = new Enemy();

		Vector3 enemyPosition = {(float)(i + 1) * 20, 2.0f, 0.0f};

		newEnemy->Initialize(enemyModel_, &camera_, enemyPosition);
		enemies_.push_back(newEnemy);
	}

	// カメラコントローラ
	cameraController_ = new CameraController();
	cameraController_->SetCamera(&camera_);
	cameraController_->Initialize();
	cameraController_->SetTarget(player_);
	cameraController_->Reset();

	// 天球の生成
	skydome_ = new Skydome();
	skydome_->Initialize(skydomeModel_, textureHandle_, &camera_);

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);
}

void GameScene::Update() {
	// 自キャラの更新
	player_->Update();

	for (Enemy* enemy : enemies_) {
		enemy->Update();
	}
	CheckAllCollisions();

	// デスパーティクルの更新
	if (deathParticles_) {
		deathParticles_->Update();
	}
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
		// カメラコントローラーの更新
		cameraController_->Update();
		camera_.UpdateMatrix();
		camera_.TransferMatrix();
	}
}

void GameScene::Draw() {
	// 描画
	player_->Draw();
	skydome_->Draw();
	// デスパーティクルの描画
	if (deathParticles_) {
		deathParticles_->Draw();
	}
	for (Enemy* enemy : enemies_) {
		enemy->Draw();
	}

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
	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVertical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	worldTransformBlocks_.resize(numBlockVirtical);
	for (uint32_t i = 0; i < numBlockVirtical; i++) {
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}

	// ブロックの生成
	for (uint32_t i = 0; i < numBlockVirtical; i++) {
		for (uint32_t j = 0; j < numBlockHorizontal; j++) {
			// MapChipField からマップチップのタイプを取得
			MapChipField::MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(j, i);

			if (mapChipType != MapChipField::MapChipType::kBlank) {
				worldTransformBlocks_[i][j] = new WorldTransform();
				worldTransformBlocks_[i][j]->Initialize();
				worldTransformBlocks_[i][j]->translation_.x = kBlockWidth * j;
				worldTransformBlocks_[i][j]->translation_.y = kBlockHeight * (numBlockVirtical - 1 - i);
			} else {
				// ブロックがない場合は nullptr を設定
				worldTransformBlocks_[i][j] = nullptr;
			}
		}
	}
}

void GameScene::CheckAllCollisions() {
	AABB aabb1, aabb2;
	aabb1 = player_->GetAABB();

	for (Enemy* enemy : enemies_) {
		aabb2 = enemy->GetAABB();
		if (AABBCollision(aabb1, aabb2)) {
			player_->OnCollision(enemy);
			enemy->OnCollision(player_);
		}
	}


}
