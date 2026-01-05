#include "GameScene.h"
#include "Enemy.h"
#include "Explosion.h"
#include "Ground.h"
#include "IScene.h"
#include "KamataEngine.h"
#include "Player.h"
#include "ResultScene.h"
#include "Reticle.h"
#include "json.hpp"
#include "mathStruct.h"
#include <assert.h>
#include <cmath>
#include <fstream>

using json = nlohmann::json;
using namespace KamataEngine;

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
	delete playerMissileModel_;
	delete explosionModel_;
	delete groundModel_;
	delete ground_;

	delete player_;
	delete debugCamera_;
	for (Enemy* e : enemies_)
		delete e;
	for (Explosion* e : explosions_)
		delete e;

	delete fadeSprite_;
	delete reticle_;
	delete lockOnMark_;
	delete hpBarSprite_;
	delete lifeIconSprite_;
}

void GameScene::Initialize() {
	playerModel_ = Model::CreateFromOBJ("player");
	enemyModel_ = Model::CreateFromOBJ("enemy");
	playerBulletModel_ = Model::CreateFromOBJ("playerBullet");
	enemyBulletModel_ = Model::CreateFromOBJ("enemyBullet");
	enemyMissileModel_ = Model::CreateFromOBJ("enemyMissile");
	playerMissileModel_ = Model::CreateFromOBJ("playerMissile");
	explosionModel_ = Model::Create();

	//groundModel_ = Model::CreateFromOBJ("ground");
	groundModel_ = Model::Create();

	worldTransform_.Initialize();
	camera_.Initialize();
	camera_.translation_ = {0.0f, 2.5f, -15.0f};


	ground_ = new Ground();
	ground_->Initialize(groundModel_);

	player_ = new Player();
	player_->Initialize(playerModel_, &camera_);
	player_->SetBulletModel(playerBulletModel_);
	player_->SetMissileModel(playerMissileModel_);

	input_ = Input::GetInstance();
	debugCamera_ = new DebugCamera(1280, 720);

	reticle_ = new Reticle();
	reticle_->Initialize();

	lockOnTex_ = TextureManager::Load("lockOn.png");
	lockOnMark_ = new Sprite(lockOnTex_, {0, 0}, {64, 64}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.5f, 0.5f}, false, false);
	lockOnMark_->Initialize();

	uiTexHandle_ = TextureManager::Load("white1x1.png");
	hpBarSprite_ = new Sprite(uiTexHandle_, {50, 650}, {200, 20}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, false, false);
	hpBarSprite_->Initialize();
	lifeIconSprite_ = new Sprite(uiTexHandle_, {0, 0}, {20, 20}, {1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, false, false);
	lifeIconSprite_->Initialize();

	score_ = 0;

	// --- JSON読み込み ---
	std::ifstream file("./Resources/enemy_data.json");
	if (file.fail()) {
		assert(0 && "JSON file not found.");
	}
	json deserialized;
	file >> deserialized;

	enemySpawnList_.clear();
	for (const auto& enemyData : deserialized["enemies"]) {
		EnemySpawnData data;
		data.spawnTime = enemyData["spawnTime"];
		data.position = {enemyData["position"][0], enemyData["position"][1], enemyData["position"][2]};
		data.velocity = {enemyData["velocity"][0], enemyData["velocity"][1], enemyData["velocity"][2]};
		data.type = enemyData.value("type", "A");
		data.attackPattern = enemyData.value("attackPattern", "Normal");
		enemySpawnList_.push_back(data);
	}
	enemySpawnList_.sort([](const EnemySpawnData& a, const EnemySpawnData& b) { return a.spawnTime < b.spawnTime; });

	phase_ = ScenePhase::kFadeIn;
	fadeTimer_ = kFadeDuration_;
	gameLimitTimer_ = 60 * 30;
	gameElapsedTime_ = 0;

	fadeTextureHandle_ = TextureManager::Load("white1x1.png");
	fadeSprite_ = new Sprite(fadeTextureHandle_, {0, 0}, {1280, 720}, {1, 1, 1, 1}, {0, 0}, false, false);
	fadeSprite_->Initialize();
	fadeSprite_->SetTextureRect({0.0f, 0.0f}, {1.0f, 1.0f});
}

std::optional<SceneID> GameScene::Update() {
	if (phase_ == ScenePhase::kFadeIn)
		return UpdateFadeIn();
	if (phase_ == ScenePhase::kMain)
		return UpdateMain();
	if (phase_ == ScenePhase::kFadeOut)
		return UpdateFadeOut();
	return std::nullopt;
}

std::optional<SceneID> GameScene::UpdateFadeIn() {
	if (--fadeTimer_ <= 0)
		phase_ = ScenePhase::kMain;
	return std::nullopt;
}

std::optional<SceneID> GameScene::UpdateFadeOut() {
	if (++fadeTimer_ >= kFadeDuration_)
		return SceneID::kResult;
	return std::nullopt;
}

std::optional<SceneID> GameScene::UpdateMain() {
	gameLimitTimer_--;
	gameElapsedTime_++;


	ground_->Update();

	// 敵スポーン
	while (!enemySpawnList_.empty()) {
		const EnemySpawnData& data = enemySpawnList_.front();
		if (data.spawnTime > gameElapsedTime_)
			break;

		Enemy* newEnemy = new Enemy();
		newEnemy->SetBulletModel(enemyBulletModel_);
		newEnemy->SetPlayer(player_);
		newEnemy->Initialize(enemyModel_, data.position, data.velocity, data.type, data.attackPattern);

		enemies_.push_back(newEnemy);
		enemySpawnList_.pop_front();
	}

	player_->Update();

	enemies_.remove_if([](Enemy* e) {
		if (e->IsDead()) {
			delete e;
			return true;
		}
		return false;
	});
	for (Enemy* e : enemies_)
		e->Update();
	explosions_.remove_if([](Explosion* e) {
		if (e->IsDead()) {
			delete e;
			return true;
		}
		return false;
	});
	for (Explosion* e : explosions_)
		e->Update();

	// 当たり判定
	const float kPlayerRadius = 1.0f;
	const float kEnemyRadius = 4.0f;
	const float kBulletRadius = 0.5f;

	// 自弾 vs 敵
	for (PlayerBullet* pBullet : player_->GetBullets()) {
		for (Enemy* enemy : enemies_) {
			if (pBullet->IsDead() || enemy->IsDead())
				continue;
			if (LengthSquared(pBullet->GetWorldPosition(), enemy->GetWorldPosition()) < pow(kBulletRadius + kEnemyRadius, 2)) {
				pBullet->OnCollision();
				enemy->OnCollision(pBullet->GetPower());
				if (enemy->IsDead()) {
					score_ += 100;
					Explosion* newExp = new Explosion();
					newExp->Initialize(explosionModel_, enemy->GetWorldPosition());
					explosions_.push_back(newExp);
				}
			}
		}
	}

	// 敵弾 vs 自機
	for (Enemy* enemy : enemies_) {
		for (EnemyBullet* eBullet : enemy->GetBullets()) {
			if (eBullet->IsDead())
				continue;
			if (LengthSquared(eBullet->GetWorldPosition(), player_->GetWorldPosition()) < pow(kBulletRadius + kPlayerRadius, 2)) {
				eBullet->OnCollision();
				player_->OnCollision();
			}
		}
	}

	// ミサイル vs 敵
	for (PlayerMissile* missile : player_->GetMissiles()) {
		for (Enemy* enemy : enemies_) {
			if (missile->IsDead() || enemy->IsDead())
				continue;
			if (LengthSquared(missile->GetWorldPosition(), enemy->GetWorldPosition()) < pow(kBulletRadius + kEnemyRadius, 2)) {
				missile->OnCollision();
				enemy->OnCollision(missile->GetPower());
				if (enemy->IsDead()) {
					score_ += 500;
					Explosion* newExp = new Explosion();
					newExp->Initialize(explosionModel_, enemy->GetWorldPosition());
					explosions_.push_back(newExp);
				}
			}
		}
	}

	// 体当たり
	for (Enemy* enemy : enemies_) {
		if (enemy->IsDead())
			continue;
		if (LengthSquared(enemy->GetWorldPosition(), player_->GetWorldPosition()) < pow(kEnemyRadius + kPlayerRadius, 2)) {
			enemy->OnCollision(100);
			player_->OnCollision();
			if (enemy->IsDead()) {
				Explosion* newExp = new Explosion();
				newExp->Initialize(explosionModel_, enemy->GetWorldPosition());
				explosions_.push_back(newExp);
			}
		}
	}

	// UI更新
	float hpRatio = (float)player_->GetHP() / (float)player_->GetMaxHP();
	if (hpRatio < 0.0f)
		hpRatio = 0.0f;
	hpBarSprite_->SetSize({200.0f * hpRatio, 20.0f});

	// 終了判定
	bool isGameEnd = false;
	bool isPlayerWin = false;
	if (enemies_.empty() && enemySpawnList_.empty()) {
		isGameEnd = true;
		isPlayerWin = true;
	} else if (gameLimitTimer_ <= 0) {
		isGameEnd = true;
		isPlayerWin = false;
	} else if (player_->IsDead()) {
		isGameEnd = true;
		isPlayerWin = false;
	}

	if (isGameEnd) {
		ResultScene::isWin = isPlayerWin;
		ResultScene::finalScore = score_;
		phase_ = ScenePhase::kFadeOut;
		fadeTimer_ = 0;
	}

	// カメラ更新
	if (isDebugCameraActive_) {
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		camera_.TransferMatrix();
	} else {
		Vector3 pPos = player_->GetWorldPosition();
		Vector3 pRot = player_->GetRotation();
		Vector3 tCamPos = {pPos.x, pPos.y + 4.0f, pPos.z - 20.0f};
		camera_.translation_.x = LerpShort(camera_.translation_.x, tCamPos.x, 0.1f);
		camera_.translation_.y = LerpShort(camera_.translation_.y, tCamPos.y, 0.1f);
		camera_.translation_.z = LerpShort(camera_.translation_.z, tCamPos.z, 0.1f);
		camera_.rotation_.z = LerpShort(camera_.rotation_.z, -pRot.z * 1.0f, 0.1f);
		camera_.rotation_.x = LerpShort(camera_.rotation_.x, -pRot.x * 0.5f, 0.1f);
		camera_.UpdateMatrix();
		camera_.TransferMatrix();
	}

	// レティクル更新
	Vector3 pPos = player_->GetWorldPosition();
	Vector3 pRot = player_->GetRotation();
	Matrix4x4 matPlayer = MakeAffineMatrix({1, 1, 1}, pRot, pPos);
	Vector3 targetPos = Transform({0, 0, -50.0f}, matPlayer);
	reticle_->Update(targetPos, camera_);

	// ロックオン
	lockedEnemy_ = nullptr;
	float minDst = 100.0f;
	Vector2 rPos = reticle_->GetPosition();
	for (Enemy* e : enemies_) {
		if (e->IsDead())
			continue;
		Vector3 ePos = e->GetWorldPosition();
		if (ePos.z < camera_.translation_.z)
			continue;
		Vector2 eScr = WorldToScreen(ePos, camera_.matView, camera_.matProjection, 1280, 720);
		float dst = sqrtf(powf(eScr.x - rPos.x, 2) + powf(eScr.y - rPos.y, 2));
		if (dst < minDst) {
			minDst = dst;
			lockedEnemy_ = e;
		}
	}

	if (input_->IsTriggerMouse(1) || input_->TriggerKey(DIK_V)) {
		if (lockedEnemy_)
			player_->FireMissile(lockedEnemy_);
	}

	return std::nullopt;
}

void GameScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	if (phase_ != ScenePhase::kFadeIn) {
		Model::PreDraw(dxCommon->GetCommandList());

		//地面の描画
		ground_->Draw(camera_);

		player_->Draw();
		for (Enemy* e : enemies_)
			if (!e->IsDead())
				e->Draw(camera_);
		for (Explosion* e : explosions_)
			e->Draw(camera_);
		Model::PostDraw();
	}

	Sprite::PreDraw(dxCommon->GetCommandList(), Sprite::BlendMode::kNormal);
	if (lockedEnemy_ && !lockedEnemy_->IsDead()) {
		Vector3 ePos = lockedEnemy_->GetWorldPosition();
		Vector2 sPos = WorldToScreen(ePos, camera_.matView, camera_.matProjection, 1280, 720);
		lockOnMark_->SetPosition(sPos);
		lockOnMark_->Draw();
	}
	if (reticle_)
		reticle_->Draw();
	if (hpBarSprite_)
		hpBarSprite_->Draw();

	int lives = player_->GetLives();
	for (int i = 0; i < lives; i++) {
		lifeIconSprite_->SetPosition({50.0f + i * 25.0f, 620.0f});
		lifeIconSprite_->Draw();
	}

#ifdef _DEBUG
	ImGui::Begin("HUD");
	ImGui::Text("SCORE: %06d", score_);
	ImGui::Text("HP: %d / %d", player_->GetHP(), player_->GetMaxHP());
	ImGui::Text("LIVES: %d", player_->GetLives());
	ImGui::End();
#endif

	float alpha = 0.0f;
	if (phase_ == ScenePhase::kFadeIn)
		alpha = (float)fadeTimer_ / kFadeDuration_;
	else if (phase_ == ScenePhase::kFadeOut)
		alpha = (float)fadeTimer_ / kFadeDuration_;
	if (alpha > 0.0f && fadeSprite_) {
		fadeSprite_->SetColor({1, 1, 1, alpha});
		fadeSprite_->Draw();
	}
	Sprite::PostDraw();
}