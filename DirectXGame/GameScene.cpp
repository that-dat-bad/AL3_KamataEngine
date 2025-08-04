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
	delete attackFxModel_;
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
	delete fade_;
}

void GameScene::Initialize() {
	isInitialized_ = true;
	// ファイル名を指定してテクスチャを読み込む
	textureHandle_ = TextureManager::Load("UVchecker.png");

	// 3Dモデルの生成
	playerModel_ = Model::CreateFromOBJ("player", true);
	blockModel_ = Model::Create();
	skydomeModel_ = Model::CreateFromOBJ("ball", true);
	enemyModel_ = Model::CreateFromOBJ("enemy", true);
	deathParticleModel_ = Model::CreateFromOBJ("deathParticle", true);
	attackFxModel_ = Model::CreateFromOBJ("attackFX", true);

	// マップチップフィールドの生成と初期化
	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");
	GenerateBlocks();

	// 自キャラ生成
	player_ = new Player();
	// 座標をマップチップ番号で指定
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(0, 18);
	// 自キャラの初期化
	player_->Initialize(playerModel_, attackFxModel_, &camera_, playerPosition);

	player_->SetMapChipField(mapChipField_);

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
	// フェードの生成・初期化
	fade_ = new Fade();
	fade_->Initialize();
	// フェードインから開始
	phase_ = Phase::kFadeIn;
	fade_->Start(Fade::Status::FadeIn, 1.0f); // 1秒でフェードイン
}

void GameScene::Update() {

	// フェーズごとの更新
	switch (phase_) {
	case Phase::kFadeIn:
		UpdateFadeInPhase(); // フェードインフェーズの更新
		break;
	case Phase::kPlay:
		UpdatePlayPhase(); // ゲームプレイフェーズの更新
		break;
	case Phase::kDeath:
		UpdateDeathPhase(); // デス演出フェーズの更新
		break;
	case Phase::kFadeOut:
		fade_->Update();
		if (fade_->IsFinished()) {
			finished_ = true; // シーン終了
		}
		break;
	}

	// フェーズの切り替え
	ChangePhase();
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
	if (!isInitialized_) {
		return;
	}

	// 常に表示されるオブジェクト
	// --- 天球 ---
	skydome_->Draw();

	// --- ブロック ---
	Model::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	for (auto& row : worldTransformBlocks_) {
		for (WorldTransform* block : row) {
			if (block) {
				blockModel_->Draw(*block, camera_);
			}
		}
	}
	Model::PostDraw();

	// --- 敵キャラ ---
	for (Enemy* enemy : enemies_) {
		enemy->Draw();
	}

	// --- プレイヤー ---
	// 死亡演出中は非表示にする
	if (phase_ != Phase::kDeath) {
		player_->Draw();
	}

	// --- フェーズごとの特別な描画 ---
	switch (phase_) {
	case Phase::kDeath:
		if (deathParticles_) {
			deathParticles_->Draw();
		}
		break;
	}

	// --- フェード ---
	// フェードインとフェードアウト中のみ描画
	if (phase_ == Phase::kFadeIn || phase_ == Phase::kFadeOut) {
		fade_->Draw();
	}
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
		if (enemy->IsCollisionDisabled()) {
			continue;
		}
		aabb2 = enemy->GetAABB();
		if (AABBCollision(aabb1, aabb2)) {
			player_->OnCollision(enemy);
			enemy->OnCollision(player_);
		}
	}
}

#pragma region フェーズごとの処理

void GameScene::UpdateFadeInPhase() {
	player_->Update();
	fade_->Update();
	for (Enemy* enemy : enemies_) {
		enemy->Update();
	}
	for (auto& row : worldTransformBlocks_) {
		for (WorldTransform* block : row) {
			if (block) {
				block->matWorld_ = MakeAffineMatrix(block->scale_, block->rotation_, block->translation_);
				block->TransferMatrix();
			}
		}
	}
	if (fade_->IsFinished()) {
		phase_ = Phase::kPlay;
	}
}

void GameScene::UpdatePlayPhase() {
	// 天球の更新
	skydome_->Update();
	// 自キャラの更新
	player_->Update();
	// 敵キャラの更新
	for (Enemy* enemy : enemies_) {
		enemy->Update();
	}
	enemies_.remove_if([](Enemy* enemy) {
		if (enemy->IsDead()) {
			delete enemy;
			return true;
			
		}
		return false;
		
	});
	// カメラコントローラーの更新
	cameraController_->Update();
	// 全ての当たり判定
	CheckAllCollisions();
	// ブロックの更新
	for (auto& row : worldTransformBlocks_) {
		for (WorldTransform* block : row) {
			if (block) {
				block->matWorld_ = MakeAffineMatrix(block->scale_, block->rotation_, block->translation_);
				block->TransferMatrix();
			}
		}
	}
}

void GameScene::UpdateDeathPhase() {
	// 天球の更新
	skydome_->Update();
	// 敵キャラの更新
	for (Enemy* enemy : enemies_) {
		enemy->Update();
	}
	// デスパーティクルの更新
	if (deathParticles_) {
		deathParticles_->Update();
		// パーティクル演出が終わったら、このシーンを終了状態にする
		if (deathParticles_->IsFinished()) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, 1.0f);
		}
	}
	// カメラの更新 (カメラコントローラーは呼ばない)
	camera_.UpdateMatrix();
	// ブロックの更新
	for (auto& row : worldTransformBlocks_) {
		for (WorldTransform* block : row) {
			if (block) {
				block->matWorld_ = MakeAffineMatrix(block->scale_, block->rotation_, block->translation_);
				block->TransferMatrix();
			}
		}
	}
}

void GameScene::ChangePhase() {
	switch (phase_) {
	case Phase::kPlay:
		// プレイヤーが死んだらデス演出フェーズに切り替え
		if (player_->IsDead()) {
			phase_ = Phase::kDeath;
			// パーティクルを生成
			delete deathParticles_;
			deathParticles_ = new DeathParticles();
			deathParticles_->Initialize(deathParticleModel_, &camera_, player_->GetWorldPosition());
		}
		break;
	case Phase::kDeath:
		// デス演出から他のフェーズへの切り替えは今回実装しない
		break;
	}
}

#pragma endregion