#include "GameScene.h"
#include "HitEffect.h"
#include "mathStruct.h"
#include <queue>
#include <random>
#include <string>
using namespace KamataEngine;

GameScene::~GameScene() {
	delete player_;
	delete blockModel_;
	delete skydomeModel_;
	delete skydome_;
	delete attackFxModelRight_;
	delete attackFxModelLeft_;
	delete hitEffectModel_;
	delete lockedDoorModel_;
	delete keyModel_;
	delete goalModel_;
	// UIアイコンの解放
	for (int i = 0; i < kMaxKeyIcons; ++i) {
		delete keyIcons_[i];
		delete keyIconsEmpty_[i];
	}
	for (auto& row : blocks_) {
		for (const BlockData& blockData : row) {
			delete blockData.worldTransform;
			delete blockData.objectColor;
		}
		row.clear();
	}
	blocks_.clear();
	delete debugCamera_;
	delete mapChipField_;
	delete cameraController_;
	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	enemies_.clear();
	delete deathParticles_;
	delete deathParticleModel_;
	delete fade_;
	for (HitEffect* effect : hitEffects_) {
		delete effect;
	}
	hitEffects_.clear();
}

void GameScene::Initialize(int stageNumber) {
	isInitialized_ = true;
	textureHandle_ = TextureManager::Load("UVchecker.png");

	playerModel_ = Model::CreateFromOBJ("player", true);
	blockModel_ = Model::Create();
	skydomeModel_ = Model::CreateFromOBJ("ball", true);
	enemyModel_ = Model::CreateFromOBJ("enemy", true);
	deathParticleModel_ = Model::CreateFromOBJ("deathParticle", true);
	attackFxModelRight_ = Model::CreateFromOBJ("attackFXRight", true);
	attackFxModelLeft_ = Model::CreateFromOBJ("attackFXLeft", true);
	hitEffectModel_ = Model::CreateFromOBJ("hitFX", true);
	lockedDoorModel_ = Model::CreateFromOBJ("lock", true);
	keyModel_ = Model::CreateFromOBJ("key", true);
	goalModel_ = Model::CreateFromOBJ("key", true);

	// --- UIの初期化 ---
	uint32_t keyIconTexture = TextureManager::Load("png/key_icon.png");
	uint32_t keyIconEmptyTexture = TextureManager::Load("png/key_icon_empty.png");
	for (int i = 0; i < kMaxKeyIcons; ++i) {
		// 表示座標を横にずらしながら3つ生成
		float posX = 20.0f + (i * 74.0f); // アイコンの幅に合わせて調整
		keyIcons_[i] = Sprite::Create(keyIconTexture, {posX, 20.0f});
		keyIconsEmpty_[i] = Sprite::Create(keyIconEmptyTexture, {posX, 20.0f});
	}

	HitEffect::SetModel(hitEffectModel_);
	HitEffect::SetCamera(&camera_);

	mapChipField_ = new MapChipField;
	std::string mapFileName = "Resources/csv/level" + std::to_string(stageNumber) + ".csv";
	mapChipField_->LoadMapChipCsv(mapFileName);
	GenerateBlocks();

	player_ = new Player();
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(1, 18);
	player_->Initialize(playerModel_, attackFxModelRight_, attackFxModelLeft_, textureHandleAttackFX_, &camera_, playerPosition);
	player_->SetMapChipField(mapChipField_);

	kEnemyCount_ = 0;
	for (int32_t i = 0; i < kEnemyCount_; i++) {
		Enemy* newEnemy = new Enemy();
		Vector3 enemyPosition = {(float)(i + 1) * 20, 2.0f, 0.0f};
		newEnemy->Initialize(enemyModel_, &camera_, enemyPosition);
		newEnemy->SetGameScene(this);
		enemies_.push_back(newEnemy);
	}

	cameraController_ = new CameraController();
	cameraController_->SetCamera(&camera_);
	cameraController_->Initialize();
	cameraController_->SetTarget(player_);
	cameraController_->Reset();

	skydome_ = new Skydome();
	skydome_->Initialize(skydomeModel_, textureHandle_, &camera_);

	debugCamera_ = new DebugCamera(1280, 720);
	fade_ = new Fade();
	fade_->Initialize();
	phase_ = Phase::kFadeIn;
	fade_->Start(Fade::Status::FadeIn, 1.0f);
}

void GameScene::Update() {
	switch (phase_) {
	case Phase::kFadeIn:
		UpdateFadeInPhase();
		break;
	case Phase::kPlay:
		UpdatePlayPhase();
		break;
	case Phase::kDeath:
		UpdateDeathPhase();
		break;
	case Phase::kFadeOut:
		fade_->Update();
		if (fade_->IsFinished()) {
			finished_ = true;
		}
		break;
	}

	ChangePhase();
#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}
#endif

	if (isDebugCameraActive_) {
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		camera_.TransferMatrix();
	} else {
		cameraController_->Update();
		camera_.UpdateMatrix();
		camera_.TransferMatrix();
	}
}

void GameScene::Draw() {
	if (!isInitialized_) {
		return;
	}

	skydome_->Draw();
	Model::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	Player::PlayerColor playerColor = player_->GetCurrentColor();
	for (auto& row : blocks_) {
		for (const BlockData& blockData : row) {
			if (blockData.worldTransform) {
				bool isVisible = false;

				if (blockData.type == MapChipField::MapChipType::kBlock || blockData.type >= MapChipField::MapChipType::kCurtain_SetRed || blockData.type == MapChipField::MapChipType::kKey ||
				    blockData.type == MapChipField::MapChipType::kLockedDoor || blockData.type == MapChipField::MapChipType::kGoal) {
					isVisible = true;
				} else {
					if (playerColor == Player::PlayerColor::kNormal) {
						isVisible = (blockData.type == MapChipField::MapChipType::kBlock);
					} else {
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
					Model* modelToDraw = blockModel_;
					if (blockData.type == MapChipField::MapChipType::kLockedDoor) {
						modelToDraw = lockedDoorModel_;
					} else if (blockData.type == MapChipField::MapChipType::kKey) {
						modelToDraw = keyModel_;
					} else if (blockData.type == MapChipField::MapChipType::kGoal) {
						modelToDraw = goalModel_;
					}
					modelToDraw->Draw(*blockData.worldTransform, camera_, blockData.objectColor);
				}
			}
		}
	}
	Model::PostDraw();

	for (Enemy* enemy : enemies_) {
		enemy->Draw();
	}
	for (HitEffect* effect : hitEffects_) {
		effect->Draw();
	}
	if (phase_ != Phase::kDeath) {
		player_->Draw();
	}
	switch (phase_) {
	case Phase::kDeath:
		if (deathParticles_) {
			deathParticles_->Draw();
		}
		break;
	}

	// --- UIの描画 ---
	Sprite::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	int keyCount = player_->GetKeyCount();
	for (int i = 0; i < kMaxKeyIcons; ++i) {
		if (i < keyCount) {
			keyIcons_[i]->Draw();
		} else {
			keyIconsEmpty_[i]->Draw();
		}
	}
	Sprite::PostDraw();

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

	for (uint32_t i = 0; i < numBlockVirtical; i++) {
		for (uint32_t j = 0; j < numBlockHorizontal; j++) {
			MapChipField::MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(j, i);

			if (mapChipType != MapChipField::MapChipType::kBlank) {
				WorldTransform* newWorldTransform = new WorldTransform();
				newWorldTransform->Initialize();
				newWorldTransform->translation_.x = kBlockWidth * j;
				newWorldTransform->translation_.y = kBlockHeight * (numBlockVirtical - 1 - i);

				ObjectColor* newObjectColor = new ObjectColor();
				newObjectColor->Initialize();

				switch (mapChipType) {
				case MapChipField::MapChipType::kBlock_Red:
					newObjectColor->SetColor({1.0f, 0.2f, 0.2f, 1.0f});
					break;
				case MapChipField::MapChipType::kBlock_Green:
					newObjectColor->SetColor({0.2f, 1.0f, 0.2f, 1.0f});
					break;
				case MapChipField::MapChipType::kBlock_Blue:
					newObjectColor->SetColor({0.2f, 0.2f, 1.0f, 1.0f});
					break;
				case MapChipField::MapChipType::kCurtain_SetRed:
					newObjectColor->SetColor({1.0f, 0.2f, 0.2f, 0.5f});
					break;
				case MapChipField::MapChipType::kCurtain_SetGreen:
					newObjectColor->SetColor({0.2f, 1.0f, 0.2f, 0.5f});
					break;
				case MapChipField::MapChipType::kCurtain_SetBlue:
					newObjectColor->SetColor({0.2f, 0.2f, 1.0f, 0.5f});
					break;
				case MapChipField::MapChipType::kBlock:
				case MapChipField::MapChipType::kKey:
				case MapChipField::MapChipType::kLockedDoor:
				case MapChipField::MapChipType::kGoal:
				default:
					newObjectColor->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
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

void GameScene::OpenConnectedDoors(uint32_t startX, uint32_t startY) {
	std::queue<MapChipField::IndexSet> searchQueue;
	searchQueue.push({startX, startY});

	std::vector<std::vector<bool>> searched(mapChipField_->GetNumBlockVertical(), std::vector<bool>(mapChipField_->GetNumBlockHorizontal(), false));
	searched[startY][startX] = true;

	while (!searchQueue.empty()) {
		MapChipField::IndexSet currentIndex = searchQueue.front();
		searchQueue.pop();

		mapChipField_->mapChipData_.data[currentIndex.yIndex][currentIndex.xIndex] = MapChipField::MapChipType::kBlank;
		delete blocks_[currentIndex.yIndex][currentIndex.xIndex].worldTransform;
		blocks_[currentIndex.yIndex][currentIndex.xIndex].worldTransform = nullptr;
		delete blocks_[currentIndex.yIndex][currentIndex.xIndex].objectColor;
		blocks_[currentIndex.yIndex][currentIndex.xIndex].objectColor = nullptr;

		int dx[] = {0, 0, -1, 1};
		int dy[] = {-1, 1, 0, 0};

		for (int i = 0; i < 4; ++i) {
			uint32_t nextX = currentIndex.xIndex + dx[i];
			uint32_t nextY = currentIndex.yIndex + dy[i];

			if (nextX >= mapChipField_->GetNumBlockHorizontal() || nextY >= mapChipField_->GetNumBlockVertical()) {
				continue;
			}

			if (!searched[nextY][nextX] && mapChipField_->mapChipData_.data[nextY][nextX] == MapChipField::MapChipType::kLockedDoor) {
				searchQueue.push({nextX, nextY});
				searched[nextY][nextX] = true;
			}
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
	skydome_->Update();
	player_->Update();
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
	hitEffects_.remove_if([](HitEffect* effect) {
		if (effect->IsDead()) {
			delete effect;
			return true;
		}
		return false;
	});
	cameraController_->Update();

	AABB playerAABB = player_->GetAABB();
	MapChipField::IndexSet indexMin = mapChipField_->GetMapChipIndexSetByPosition(playerAABB.min);
	MapChipField::IndexSet indexMax = mapChipField_->GetMapChipIndexSetByPosition(playerAABB.max);
	uint32_t mapWidth = mapChipField_->GetNumBlockHorizontal();
	uint32_t mapHeight = mapChipField_->GetNumBlockVertical();
	uint32_t startX = (std::max)(0u, indexMin.xIndex);
	uint32_t endX = (std::min)(mapWidth - 1, indexMax.xIndex);
	uint32_t startY = (std::max)(0u, indexMax.yIndex);
	uint32_t endY = (std::min)(mapHeight - 1, indexMin.yIndex);

	bool keyUsedThisFrame = false;
	for (uint32_t y = startY; y <= endY; ++y) {
		for (uint32_t x = startX; x <= endX; ++x) {
			MapChipField::MapChipType& chipType = mapChipField_->mapChipData_.data[y][x];

			if (chipType == MapChipField::MapChipType::kKey) {
				player_->AddKey();
				chipType = MapChipField::MapChipType::kBlank;
				delete blocks_[y][x].worldTransform;
				blocks_[y][x].worldTransform = nullptr;
				delete blocks_[y][x].objectColor;
				blocks_[y][x].objectColor = nullptr;
			}
			if (chipType == MapChipField::MapChipType::kLockedDoor && player_->GetKeyCount() > 0) {
				player_->UseKey();
				OpenConnectedDoors(x, y);
				keyUsedThisFrame = true;
				break;
			}
			if (chipType == MapChipField::MapChipType::kGoal) {
				isCleared_ = true;
				finished_ = true;
			}
		}
		if (keyUsedThisFrame) {
			break;
		}
	}

	CheckAllCollisions();
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
	skydome_->Update();
	for (Enemy* enemy : enemies_) {
		enemy->Update();
	}
	if (deathParticles_) {
		deathParticles_->Update();
		if (deathParticles_->IsFinished()) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, 1.0f);
		}
	}
	camera_.UpdateMatrix();
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
		if (player_->IsDead()) {
			phase_ = Phase::kDeath;
			delete deathParticles_;
			deathParticles_ = new DeathParticles();
			deathParticles_->Initialize(deathParticleModel_, &camera_, player_->GetWorldPosition());
		}
		break;
	case Phase::kDeath:
		break;
	}
}
#pragma endregion