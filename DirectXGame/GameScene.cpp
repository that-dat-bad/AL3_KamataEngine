#include "GameScene.h"
#include "Enemy.h"
#include "IScene.h"
#include "KamataEngine.h"
#include "Player.h"
#include "ResultScene.h"
#include <assert.h>
#include <cmath> // 距離計算用

// ★追加: JSONとファイル読み込み用
#include "json.hpp"
#include <fstream>
using json = nlohmann::json;

using namespace KamataEngine;

// 距離の2乗を計算するヘルパー関数
float LengthSquared(const Vector3& v1, const Vector3& v2) {
	float dx = v1.x - v2.x;
	float dy = v1.y - v2.y;
	float dz = v1.z - v2.z;
	return dx * dx + dy * dy + dz * dz;
}

GameScene::~GameScene() {
	delete playerModel_;
	delete enemyModel_;
	delete playerBulletModel_;
	delete enemyBulletModel_;
	delete player_;
	delete debugCamera_;
	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	delete fadeSprite_;
}

void GameScene::Initialize() {
	// textureHandle_ = TextureManager::Load("UVChecker.png");
	playerModel_ = Model::CreateFromOBJ("player");
	enemyModel_ = Model::CreateFromOBJ("enemy");
	playerBulletModel_ = Model::Create();
	enemyBulletModel_ = Model::Create();

	worldTransform_.Initialize();
	camera_.Initialize();

	camera_.translation_ = {0.0f, 2.5f, -15.0f};

	player_ = new Player();
	player_->Initialize(playerModel_, &camera_);
	player_->SetBulletModel(playerBulletModel_);

	input_ = Input::GetInstance();
	debugCamera_ = new DebugCamera(1280, 720);

	// ==================================================
	// ★変更: JSONファイルから敵データを読み込んで生成
	// ==================================================
	std::ifstream file("./Resources/enemy_data.json");
	if (file.fail()) {
		assert(0 && "JSON file not found. Please check Resources folder.");
	}

	json deserialized;
	file >> deserialized;

	// "enemies" 配列をループして生成
	for (const auto& enemyData : deserialized["enemies"]) {
		// 座標取得
		Vector3 position;
		position.x = enemyData["position"][0];
		position.y = enemyData["position"][1];
		position.z = enemyData["position"][2];

		// 速度取得
		Vector3 velocity;
		velocity.x = enemyData["velocity"][0];
		velocity.y = enemyData["velocity"][1];
		velocity.z = enemyData["velocity"][2];

		// 敵の生成
		Enemy* newEnemy = new Enemy();
		newEnemy->SetBulletModel(enemyBulletModel_);

		// Initializeに速度も渡す
		newEnemy->Initialize(enemyModel_, position, velocity);

		// プレイヤー情報をセット
		newEnemy->SetPlayer(player_);

		enemies_.push_back(newEnemy);
	}
	// ==================================================

	phase_ = ScenePhase::kFadeIn;
	fadeTimer_ = kFadeDuration_;

	// 制限時間の設定 (例: 60fps * 30秒 = 1800)
	gameTimer_ = 60 * 30;

	fadeTextureHandle_ = TextureManager::Load("white1x1.png");
	Vector2 position = {0.0f, 0.0f};
	Vector2 size = {1280.0f, 720.0f};
	Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};
	Vector2 anchorpoint = {0.0f, 0.0f};

	fadeSprite_ = new Sprite(fadeTextureHandle_, position, size, color, anchorpoint, false, false);
	fadeSprite_->Initialize();
	fadeSprite_->SetTextureRect({0.0f, 0.0f}, {1.0f, 1.0f});
}

std::optional<SceneID> GameScene::Update() {
	switch (phase_) {
	case ScenePhase::kFadeIn:
		return UpdateFadeIn();
	case ScenePhase::kMain:
		return UpdateMain();
	case ScenePhase::kFadeOut:
		return UpdateFadeOut();
	}
	return std::nullopt;
}

std::optional<SceneID> GameScene::UpdateFadeIn() {
	fadeTimer_--;
	if (fadeTimer_ <= 0) {
		phase_ = ScenePhase::kMain;
	}
	return std::nullopt;
}

std::optional<SceneID> GameScene::UpdateMain() {
	// 制限時間を減らす
	gameTimer_--;

	player_->Update();

	// 死亡した敵の削除処理
	enemies_.remove_if([](Enemy* enemy) {
		if (enemy->IsDead()) {
			delete enemy;
			return true;
		}
		return false;
	});

	for (Enemy* enemy : enemies_) {
		enemy->Update();
	}

	// =============================================
	// 当たり判定 (Collision)
	// =============================================
	const float kPlayerRadius = 1.0f;
	const float kEnemyRadius = 1.0f;
	const float kBulletRadius = 0.5f;

	// --- 1. 自弾 vs 敵 ---
	const std::list<PlayerBullet*>& playerBullets = player_->GetBullets();
	for (PlayerBullet* pBullet : playerBullets) {
		for (Enemy* enemy : enemies_) {
			if (pBullet->IsDead() || enemy->IsDead())
				continue;

			Vector3 posA = pBullet->GetWorldPosition();
			Vector3 posB = enemy->GetWorldPosition();
			float distSq = LengthSquared(posA, posB);

			float hitRadius = kBulletRadius + kEnemyRadius;
			if (distSq < hitRadius * hitRadius) {
				pBullet->OnCollision();
				enemy->OnCollision(); // 敵死亡
			}
		}
	}

	// --- 2. 敵弾 vs プレイヤー ---
	for (Enemy* enemy : enemies_) {
		const std::list<EnemyBullet*>& enemyBullets = enemy->GetBullets();
		for (EnemyBullet* eBullet : enemyBullets) {
			if (eBullet->IsDead())
				continue;

			Vector3 posA = eBullet->GetWorldPosition();
			Vector3 posB = player_->GetWorldPosition();
			float distSq = LengthSquared(posA, posB);

			float hitRadius = kBulletRadius + kPlayerRadius;
			if (distSq < hitRadius * hitRadius) {
				eBullet->OnCollision();
				player_->OnCollision();
			}
		}
	}

	// =============================================
	// 勝敗判定
	// =============================================

	// パターンA: 敵が全滅していたら「WIN」
	if (enemies_.empty()) {
		ResultScene::isWin = true; // 勝ちフラグ
		phase_ = ScenePhase::kFadeOut;
		fadeTimer_ = 0;
	}

	// パターンB: 制限時間が尽きたら「LOSE」
	else if (gameTimer_ <= 0) {
		ResultScene::isWin = false; // 負けフラグ
		phase_ = ScenePhase::kFadeOut;
		fadeTimer_ = 0;
	}

#ifdef _DEBUG
	if (input_->TriggerKey(DIK_0)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}
	if (isDebugCameraActive_) {
		ImGui::Begin("Debug Camera");
		ImGui::Text("Debug Camera: ON");
		ImGui::End();
	}
#endif

	if (isDebugCameraActive_) {
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		camera_.TransferMatrix();
	} else {
		camera_.UpdateMatrix();
		camera_.TransferMatrix();
	}

	return std::nullopt;
}

std::optional<SceneID> GameScene::UpdateFadeOut() {
	fadeTimer_++;
	if (fadeTimer_ >= kFadeDuration_) {
		return SceneID::kResult;
	}
	return std::nullopt;
}

void GameScene::Draw() {
	KamataEngine::DirectXCommon* dxCommon = KamataEngine::DirectXCommon::GetInstance();

	if (phase_ != ScenePhase::kFadeIn) {
		KamataEngine::Model::PreDraw(dxCommon->GetCommandList());
		player_->Draw();

		for (Enemy* enemy : enemies_) {
			// フェードアウト中は死んで消えた敵を描画しない
			if (!enemy->IsDead()) {
				enemy->Draw(camera_);
			}
		}
		KamataEngine::Model::PostDraw();
	}

	float alpha = 0.0f;
	if (phase_ == ScenePhase::kFadeIn) {
		alpha = (float)fadeTimer_ / (float)kFadeDuration_;
	} else if (phase_ == ScenePhase::kFadeOut) {
		alpha = (float)fadeTimer_ / (float)kFadeDuration_;
	}

	if (alpha > 0.0f && fadeSprite_) {
		Sprite::PreDraw(dxCommon->GetCommandList(), Sprite::BlendMode::kNormal);
		fadeSprite_->SetColor({1.0f, 1.0f, 1.0f, alpha});
		fadeSprite_->Draw();
		Sprite::PostDraw();
	}
}