#include "GameScene.h"
#include "Enemy.h"
#include "IScene.h"
#include "KamataEngine.h"
#include "Player.h"
#include "ResultScene.h" // ★追加: 勝敗フラグを操作するために必要
#include <assert.h>
#include <cmath> // 距離計算用

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

	// 敵の生成
	Enemy* newEnemy = new Enemy();
	newEnemy->SetBulletModel(enemyBulletModel_);
	newEnemy->Initialize(enemyModel_, {0, 0, 50.0f});
	// プレイヤー情報をセット
	newEnemy->SetPlayer(player_);

	enemies_.push_back(newEnemy);

	phase_ = ScenePhase::kFadeIn;
	fadeTimer_ = kFadeDuration_;

	// ★追加: 制限時間の設定 (例: 60fps * 30秒 = 1800)
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
	// ★追加: 制限時間を減らす
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
	// ★追加: 勝敗判定
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

	// パターンC: プレイヤーが被弾して負ける場合の処理が必要ならここに追加
	// (今回は「時間切れ」がLOSE条件とのことなので、被弾で即LOSEかは仕様次第ですが、
	//  もし被弾で負けにするなら player_->IsDead() などを判定します)

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