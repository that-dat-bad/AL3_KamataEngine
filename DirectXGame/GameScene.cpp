#include "GameScene.h"
using namespace KamataEngine;
#include "mathStruct.h"

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
	delete cameraController_; // カメラコントローラの解放
}

void GameScene::Initialize() {
	// ファイル名を指定してテクスチャを読み込む
	textureHandle_ = TextureManager::Load("UVchecker.png");

	// 3Dモデルの生成
	playerModel_ = Model::CreateFromOBJ("player", true);
	blockModel_ = Model::Create();
	skydomeModel_ = Model::CreateFromOBJ("ball", true);

	// カメラの初期化
	camera_.Initialize();

	// 自キャラ生成
	player_ = new Player();

	// 天球の生成
	skydome_ = new Skydome();

	// 天球の初期化
	skydome_->Initialize(skydomeModel_, textureHandle_, &camera_);

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);

	// カメラコントローラの初期化
	// 生成
	cameraController_ = new CameraController();
	// 初期化
	cameraController_->Initialize();
	// 対象をセット
	cameraController_->SetTarget(player_);
	// リセット(瞬間合わせ)
	cameraController_->Reset();

	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");
	GenerateBlocks();

	// 座標をマップチップ番号で指定
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(0, 18);

	// 自キャラの初期化
	player_->Initialize(playerModel_, &camera_, playerPosition);
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
		// カメラコントローラーの更新
		cameraController_->Update();
		camera_.UpdateMatrix();
		camera_.TransferMatrix();

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
				worldTransformBlocks_[i][j]->translation_.y = kBlockHeight * (numBlockVirtical - 1 - i);
			} else {
				// ブロックがない場合は nullptr を設定
				worldTransformBlocks_[i][j] = nullptr;
			}
		}
	}
}
