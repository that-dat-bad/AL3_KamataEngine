#include "GameScene.h"
#include "HitEffect.h"
#include "mathStruct.h"
#include <random>
using namespace KamataEngine;

// デストラクタ
GameScene::~GameScene() {

	delete player_;
	delete blockModel_;
	delete skydomeModel_;
	delete skydome_;
	delete attackFxModelRight_;
	delete attackFxModelLeft_;
	delete hitEffectModel_;
	for (auto& row : blocks_) {
		for (const BlockData& blockData : row) {
			delete blockData.worldTransform; // 各ワールド変換を解放
			delete blockData.objectColor;    // 色情報も解放
		}
		row.clear(); // 行をクリア
	}
	blocks_.clear();
	delete debugCamera_; // デバッグカメラの解放
	delete mapChipField_;
	delete cameraController_; // カメラコントローラを解放

	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	enemies_.clear(); // vectorをクリア
	delete deathParticles_;
	delete deathParticleModel_;
	delete fade_;
	for (HitEffect* effect : hitEffects_) {
		delete effect;
	}
	hitEffects_.clear();
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
	attackFxModelRight_ = Model::CreateFromOBJ("attackFXRight", true);
	attackFxModelLeft_ = Model::CreateFromOBJ("attackFXLeft", true);
	hitEffectModel_ = Model::CreateFromOBJ("hitFX", true);

	HitEffect::SetModel(hitEffectModel_);
	HitEffect::SetCamera(&camera_);

	// マップチップフィールドの生成と初期化
	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");
	GenerateBlocks();

	// 自キャラ生成
	player_ = new Player();
	// 座標をマップチップ番号で指定
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(1, 18);
	// 自キャラの初期化
	player_->Initialize(playerModel_, attackFxModelRight_, attackFxModelLeft_, textureHandleAttackFX_, &camera_, playerPosition);

	player_->SetMapChipField(mapChipField_);

	// 敵キャラ生成
	kEnemyCount_ = 0; // 敵キャラの数を定義

	for (int32_t i = 0; i < kEnemyCount_; i++) {
		Enemy* newEnemy = new Enemy();

		Vector3 enemyPosition = {(float)(i + 1) * 20, 2.0f, 0.0f};

		newEnemy->Initialize(enemyModel_, &camera_, enemyPosition);
		newEnemy->SetGameScene(this);
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
	Player::PlayerColor playerColor = player_->GetCurrentColor(); // プレイヤーの現在の色を取得
	for (auto& row : blocks_) {
		for (const BlockData& blockData : row) {
			if (blockData.worldTransform) {
				// ブロックが見えるかどうかを判定
				bool isVisible = false;

				// ★★★ここから表示ルールの修正★★★

				// 最初に、マップチップがカーテンかどうかをチェック
				if (blockData.type == MapChipField::MapChipType::kCurtain_SetRed || blockData.type == MapChipField::MapChipType::kCurtain_SetGreen ||
				    blockData.type == MapChipField::MapChipType::kCurtain_SetBlue) {
					// カーテンは常に見える
					isVisible = true;
				}
				// カーテンでない場合（通常のブロックか、色のブロックの場合）
				else {
					// プレイヤーが通常色の時は、通常ブロックのみ見える
					if (playerColor == Player::PlayerColor::kNormal) {
						isVisible = (blockData.type == MapChipField::MapChipType::kBlock);
					}
					// プレイヤーが特定の色を持つ時は、通常ブロックと自分の色のブロックが見える
					else {
						if (blockData.type == MapChipField::MapChipType::kBlock) {
							isVisible = true;
						} else if (blockData.type == MapChipField::MapChipType::kBlock_Red) {
							isVisible = (playerColor == Player::PlayerColor::kRed);
						} else if (blockData.type == MapChipField::MapChipType::kBlock_Green) {
							isVisible = (playerColor == Player::PlayerColor::kGreen);
						} else if (blockData.type == MapChipField::MapChipType::kBlock_Blue) {
							isVisible = (playerColor == Player::PlayerColor::kBlue);
						}
					}
				}


				if (isVisible) {
					blockModel_->Draw(*blockData.worldTransform, camera_, blockData.objectColor);
				}
			}
		}
	}
	Model::PostDraw();

	// --- 敵キャラ ---
	for (Enemy* enemy : enemies_) {
		enemy->Draw();
	}
	for (HitEffect* effect : hitEffects_) {
		effect->Draw();
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

	blocks_.resize(numBlockVirtical);
	for (uint32_t i = 0; i < numBlockVirtical; i++) {
		blocks_[i].resize(numBlockHorizontal);
	}

	// ブロックの生成
	for (uint32_t i = 0; i < numBlockVirtical; i++) {
		for (uint32_t j = 0; j < numBlockHorizontal; j++) {
			MapChipField::MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(j, i);

			// ★空白以外のチップはすべて描画オブジェクトを生成する
			if (mapChipType != MapChipField::MapChipType::kBlank) {
				WorldTransform* newWorldTransform = new WorldTransform();
				newWorldTransform->Initialize();
				newWorldTransform->translation_.x = kBlockWidth * j;
				newWorldTransform->translation_.y = kBlockHeight * (numBlockVirtical - 1 - i);

				ObjectColor* newObjectColor = new ObjectColor();
				newObjectColor->Initialize();

				// マップチップの種類に応じて色と透明度を設定
				switch (mapChipType) {
				// --- ブロック（不透明） ---
				case MapChipField::MapChipType::kBlock_Red:
					newObjectColor->SetColor({1.0f, 0.2f, 0.2f, 1.0f}); // 赤色
					break;
				case MapChipField::MapChipType::kBlock_Green:
					newObjectColor->SetColor({0.2f, 1.0f, 0.2f, 1.0f}); // 緑色
					break;
				case MapChipField::MapChipType::kBlock_Blue:
					newObjectColor->SetColor({0.2f, 0.2f, 1.0f, 1.0f}); // 青色
					break;
				case MapChipField::MapChipType::kBlock:
					newObjectColor->SetColor({1.0f, 1.0f, 1.0f, 1.0f}); // 通常色 (白)
					break;
				// --- カーテン（半透明） ---
				case MapChipField::MapChipType::kCurtain_SetRed:
					newObjectColor->SetColor({1.0f, 0.2f, 0.2f, 0.5f}); // 赤色・半透明
					break;
				case MapChipField::MapChipType::kCurtain_SetGreen:
					newObjectColor->SetColor({0.2f, 1.0f, 0.2f, 0.5f}); // 緑色・半透明
					break;
				case MapChipField::MapChipType::kCurtain_SetBlue:
					newObjectColor->SetColor({0.2f, 0.2f, 1.0f, 0.5f}); // 青色・半透明
					break;
				}

				blocks_[i][j].worldTransform = newWorldTransform;
				blocks_[i][j].objectColor = newObjectColor;
				blocks_[i][j].type = mapChipType;
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

void GameScene::CreateHitEffect(const Vector3& position, const Vector3& rotation) {
	HitEffect* newHitEffect = HitEffect::Create(position, rotation);
	hitEffects_.push_back(newHitEffect);
}

#pragma region フェーズごとの処理

void GameScene::UpdateFadeInPhase() {
	player_->Update();
	fade_->Update();
	for (Enemy* enemy : enemies_) {
		enemy->Update();
	}
	for (auto& row : blocks_) {
		for (const BlockData& blockData : row) {
			if (blockData.worldTransform) {
				blockData.worldTransform->matWorld_ = MakeAffineMatrix(blockData.worldTransform->scale_, blockData.worldTransform->rotation_, blockData.worldTransform->translation_);
				blockData.worldTransform->TransferMatrix();
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

	for (HitEffect* effect : hitEffects_) {
		effect->Update();
	}
	// デスフラグの立ったヒットエフェクトを削除
	hitEffects_.remove_if([](HitEffect* effect) {
		if (effect->IsDead()) {
			delete effect;
			return true;
		}
		return false;
	});
	// カメラコントローラーの更新
	cameraController_->Update();
	// 全ての当たり判定
	CheckAllCollisions();
	// ブロックの更新
	for (auto& row : blocks_) {
		for (const BlockData& blockData : row) {
			if (blockData.worldTransform) {
				blockData.worldTransform->matWorld_ = MakeAffineMatrix(blockData.worldTransform->scale_, blockData.worldTransform->rotation_, blockData.worldTransform->translation_);
				blockData.worldTransform->TransferMatrix();
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
	for (auto& row : blocks_) {
		for (const BlockData& blockData : row) {
			if (blockData.worldTransform) {
				blockData.worldTransform->matWorld_ = MakeAffineMatrix(blockData.worldTransform->scale_, blockData.worldTransform->rotation_, blockData.worldTransform->translation_);
				blockData.worldTransform->TransferMatrix();
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