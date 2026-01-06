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

// 距離の二乗
float LengthSquared(const Vector3& v1, const Vector3& v2) {
	float dx = v1.x - v2.x;
	float dy = v1.y - v2.y;
	float dz = v1.z - v2.z;
	return dx * dx + dy * dy + dz * dz;
}

GameScene::~GameScene() {
	// シーン終了時にロックオン音が鳴っていたら止める
	if (isLockSoundPlayed_) {
		Audio::GetInstance()->StopWave(voiceHandleLockOn_);
	}

	// モデル解放
	delete playerModel_;
	delete enemyModel_;
	delete playerBulletModel_;
	delete enemyBulletModel_;
	delete enemyMissileModel_;
	delete playerMissileModel_;
	delete groundModel_;
	delete skydomeModel_;
	delete explosionModel_;

	// オブジェクト解放
	delete player_;
	delete ground_;
	delete debugCamera_;
	delete reticle_;
	delete lockOnMark_;
	delete hpBarSprite_;
	delete lifeIconSprite_;
	delete fadeSprite_;
	delete scorePlaceSprite_;

	// ★追加: 操作説明スプライト解放
	delete spriteGuide_;

	// 演出スプライト解放
	delete spriteWave_;
	delete spriteReady_;
	delete spriteStart_;
	delete spriteClear_;

	for (Enemy* e : enemies_)
		delete e;
	for (Explosion* e : explosions_)
		delete e;
}

void GameScene::Initialize() {
	// --- モデル生成 ---
	playerModel_ = Model::CreateFromOBJ("player");
	enemyModel_ = Model::CreateFromOBJ("enemy");
	playerBulletModel_ = Model::CreateFromOBJ("playerBullet");
	enemyBulletModel_ = Model::CreateFromOBJ("enemyBullet");
	enemyMissileModel_ = Model::CreateFromOBJ("enemyMissile");
	playerMissileModel_ = Model::CreateFromOBJ("playerMissile");

	// 爆発用モデル (sphere)
	explosionModel_ = Model::CreateFromOBJ("sphere");

	groundModel_ = Model::CreateFromOBJ("ground");
	skydomeModel_ = Model::CreateFromOBJ("Skydome");

	// --- カメラ・基本設定 ---
	worldTransform_.Initialize();
	camera_.Initialize();
	camera_.translation_ = {0.0f, 2.5f, -15.0f};
	input_ = Input::GetInstance();
	debugCamera_ = new DebugCamera(1280, 720);

	// --- オブジェクト生成 ---
	player_ = new Player();
	player_->Initialize(playerModel_, &camera_);
	player_->SetBulletModel(playerBulletModel_);
	player_->SetMissileModel(playerMissileModel_);

	ground_ = new Ground();
	ground_->Initialize(groundModel_);

	// 天球の初期化
	skydomeTransform_.Initialize();
	skydomeTransform_.matWorld_ = MakeAffineMatrix(skydomeTransform_.scale_, skydomeTransform_.rotation_, skydomeTransform_.translation_);
	skydomeTransform_.TransferMatrix();

	reticle_ = new Reticle();
	reticle_->Initialize();

	// --- スプライト・テクスチャ ---
	lockOnTex_ = TextureManager::Load("lockOn.png");
	lockOnMark_ = new Sprite(lockOnTex_, {0, 0}, {64, 64}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.5f, 0.5f}, false, false);
	lockOnMark_->Initialize();

	uiTexHandle_ = TextureManager::Load("white1x1.png");
	hpBarSprite_ = new Sprite(uiTexHandle_, {50, 650}, {200, 20}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, false, false);
	hpBarSprite_->Initialize();
	lifeIconSprite_ = new Sprite(uiTexHandle_, {0, 0}, {20, 20}, {1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, false, false);
	lifeIconSprite_->Initialize();

	// スコア表示の仮置き
	scorePlaceSprite_ = new Sprite(uiTexHandle_, {1100, 50}, {300, 40}, {1.0f, 1.0f, 1.0f, 0.5f}, {0.5f, 0.5f}, false, false);
	scorePlaceSprite_->Initialize();

	// ★追加: 操作説明画像の読み込み (ファイル名 guide.png と仮定)
	texGuide_ = TextureManager::Load("guide.png");
	// 画面右下 (1280, 720) を基準に、画像サイズ (640, 200) の半分だけ内側に配置
	// X: 1280 - 320 = 960
	// Y: 720 - 100 = 620
	spriteGuide_ = new Sprite(texGuide_, {960, 620}, {640, 200}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f}, false, false);
	spriteGuide_->Initialize();

	fadeTextureHandle_ = TextureManager::Load("white1x1.png");
	fadeSprite_ = new Sprite(fadeTextureHandle_, {0, 0}, {1280, 720}, {1, 1, 1, 1}, {0, 0}, false, false);
	fadeSprite_->Initialize();
	fadeSprite_->SetTextureRect({0.0f, 0.0f}, {1.0f, 1.0f});

	// 演出用テクスチャ
	texReady_ = TextureManager::Load("text_ready.png");
	texStart_ = TextureManager::Load("text_start.png");
	texClear_ = TextureManager::Load("text_clear.png");

	// 演出用スプライト生成 (頂いたコードの通りサイズを設定)
	// READY...
	spriteReady_ = new Sprite(texReady_, {640, 360}, {300, 60}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f}, false, false);
	spriteReady_->Initialize();

	// START!!
	spriteStart_ = new Sprite(texStart_, {640, 360}, {400, 80}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f}, false, false);
	spriteStart_->Initialize();

	// WAVE CLEAR!!
	spriteClear_ = new Sprite(texClear_, {640, 360}, {400, 80}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f}, false, false);
	spriteClear_->Initialize();

	// --- 音声読み込み ---
	soundLockOn_ = Audio::GetInstance()->LoadWave("missile_lock.wav");
	soundMissile_ = Audio::GetInstance()->LoadWave("missile_search.wav");
	soundExplosion_ = Audio::GetInstance()->LoadWave("explosion.mp3");

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
		data.wave = enemyData.value("wave", 1);
		data.spawnTime = enemyData["spawnTime"];
		data.position = {enemyData["position"][0], enemyData["position"][1], enemyData["position"][2]};
		data.velocity = {enemyData["velocity"][0], enemyData["velocity"][1], enemyData["velocity"][2]};
		data.type = enemyData.value("type", "A");
		data.attackPattern = enemyData.value("attackPattern", "Normal");
		enemySpawnList_.push_back(data);
	}
	enemySpawnList_.sort([](const EnemySpawnData& a, const EnemySpawnData& b) {
		if (a.wave != b.wave)
			return a.wave < b.wave;
		return a.spawnTime < b.spawnTime;
	});

	// --- 変数初期化 ---
	score_ = 0;
	gameLimitTimer_ = 60 * 180;

	currentWave_ = 1;
	waveState_ = WaveState::Intro;
	waveTimer_ = 0;

	phase_ = ScenePhase::kFadeIn;
	fadeTimer_ = kFadeDuration_;
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
	ground_->Update();

	// ==========================================
	// 1. 死亡・ゲームオーバー判定
	// ==========================================
	bool isGameOver = false;
	if (player_->IsDead()) {
		isGameOver = true;
	} else if (gameLimitTimer_ <= 0) {
		isGameOver = true;
	}

	if (isGameOver) {
		if (isLockSoundPlayed_) {
			Audio::GetInstance()->StopWave(voiceHandleLockOn_);
			isLockSoundPlayed_ = false;
		}

		ResultScene::isWin = false;
		ResultScene::finalScore = score_;
		phase_ = ScenePhase::kFadeOut;
		fadeTimer_ = 0;
		return std::nullopt;
	}

	// ==========================================
	// 2. ウェーブロジック
	// ==========================================
	switch (waveState_) {
	case WaveState::Intro:
		waveTimer_++;
		if (waveTimer_ > 180) { // 3秒で開始
			waveState_ = WaveState::Battle;
			waveTimer_ = 0;
		}
		break;

	case WaveState::Battle:
		waveTimer_++;
		{
			auto it = enemySpawnList_.begin();
			while (it != enemySpawnList_.end()) {
				if (it->wave != currentWave_)
					break;
				if (it->spawnTime > waveTimer_)
					break;

				Enemy* newEnemy = new Enemy();
				newEnemy->SetBulletModel(enemyBulletModel_);
				newEnemy->SetMissileModel(enemyMissileModel_);
				newEnemy->SetPlayer(player_);
				newEnemy->Initialize(enemyModel_, it->position, it->velocity, it->type, it->attackPattern);
				enemies_.push_back(newEnemy);

				it = enemySpawnList_.erase(it);
			}
		}

		{
			bool isSpawnFinished = true;
			for (const auto& d : enemySpawnList_) {
				if (d.wave == currentWave_) {
					isSpawnFinished = false;
					break;
				}
			}
			if (enemies_.empty() && isSpawnFinished) {
				waveState_ = WaveState::Clear;
				waveTimer_ = 0;
			}
		}
		break;

	case WaveState::Clear:
		waveTimer_++;
		if (waveTimer_ > 120) {
			bool hasNextWave = false;
			for (const auto& d : enemySpawnList_) {
				if (d.wave > currentWave_) {
					hasNextWave = true;
					break;
				}
			}
			if (hasNextWave) {
				currentWave_++;
				waveState_ = WaveState::Intro;
				waveTimer_ = 0;
			} else {
				if (isLockSoundPlayed_) {
					Audio::GetInstance()->StopWave(voiceHandleLockOn_);
					isLockSoundPlayed_ = false;
				}

				ResultScene::isWin = true;
				ResultScene::finalScore = score_;
				phase_ = ScenePhase::kFadeOut;
				fadeTimer_ = 0;
			}
		}
		break;
	}

	// ==========================================
	// 3. ゲーム更新処理
	// ==========================================
	bool isPlayerActive = (waveState_ == WaveState::Battle) && !player_->IsDead();

	player_->Update(isPlayerActive);

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

	// --- 当たり判定 ---
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
					Audio::GetInstance()->PlayWave(soundExplosion_);
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
		for (EnemyMissile* eMissile : enemy->GetMissiles()) {
			if (eMissile->IsDead())
				continue;
			if (LengthSquared(eMissile->GetWorldPosition(), player_->GetWorldPosition()) < pow(kBulletRadius + kPlayerRadius, 2)) {
				eMissile->OnCollision();
				player_->OnCollision();
				Explosion* newExp = new Explosion();
				newExp->Initialize(explosionModel_, eMissile->GetWorldPosition());
				explosions_.push_back(newExp);
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
					Audio::GetInstance()->PlayWave(soundExplosion_);
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
				Audio::GetInstance()->PlayWave(soundExplosion_);
				Explosion* newExp = new Explosion();
				newExp->Initialize(explosionModel_, enemy->GetWorldPosition());
				explosions_.push_back(newExp);
			}
		}
	}

	// --- カメラ更新 ---
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

		float tRoll = -pRot.z * 1.0f;
		float tPitch = -pRot.x * 0.5f;
		camera_.rotation_.z = LerpShort(camera_.rotation_.z, tRoll, 0.1f);
		camera_.rotation_.x = LerpShort(camera_.rotation_.x, tPitch, 0.1f);

		camera_.UpdateMatrix();
		camera_.TransferMatrix();
	}

	// --- ロックオン ---
	Vector3 pPos = player_->GetWorldPosition();
	Vector3 pRot = player_->GetRotation();
	Matrix4x4 matPlayer = MakeAffineMatrix({1, 1, 1}, pRot, pPos);
	Vector3 targetPos = Transform({0, 0, -50.0f}, matPlayer);
	reticle_->Update(targetPos, camera_);

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

	// ロックオン音の制御
	if (lockedEnemy_) {
		if (!isLockSoundPlayed_) {
			voiceHandleLockOn_ = Audio::GetInstance()->PlayWave(soundLockOn_, true);
			isLockSoundPlayed_ = true;
		}
	} else {
		if (isLockSoundPlayed_) {
			Audio::GetInstance()->StopWave(voiceHandleLockOn_);
			isLockSoundPlayed_ = false;
		}
	}

	if (isPlayerActive) {
		if (input_->IsTriggerMouse(1)) {
			if (lockedEnemy_) {
				player_->FireMissile(lockedEnemy_);
				Audio::GetInstance()->PlayWave(soundMissile_);
			}
		}
	}

	// UI
	float hpRatio = (float)player_->GetHP() / (float)player_->GetMaxHP();
	if (hpRatio < 0.0f)
		hpRatio = 0.0f;
	hpBarSprite_->SetSize({200.0f * hpRatio, 20.0f});

	return std::nullopt;
}

void GameScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	if (phase_ != ScenePhase::kFadeIn) {
		Model::PreDraw(dxCommon->GetCommandList());
		// 天球を描画 (地面より先に描く)
		if (skydomeModel_) {
			skydomeModel_->Draw(skydomeTransform_, camera_);
		}
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

	// UI描画
	if (hpBarSprite_) {
		hpBarSprite_->SetPosition({50.0f, 680.0f});
		hpBarSprite_->Draw();
	}
	int lives = player_->GetLives();
	for (int i = 0; i < lives; i++) {
		lifeIconSprite_->SetPosition({50.0f + i * 40.0f, 640.0f});
		lifeIconSprite_->Draw();
	}

	// スコア表示（仮置き）
	if (scorePlaceSprite_) {
		scorePlaceSprite_->Draw();
	}

	// ★追加: 操作説明スプライト
	if (spriteGuide_) {
		spriteGuide_->Draw();
	}

	// 演出スプライト
	if (waveState_ == WaveState::Intro) {
		if (waveTimer_ < 120) {
			if (spriteReady_)
				spriteReady_->Draw();
		} else {
			if (spriteStart_)
				spriteStart_->Draw();
		}
	} else if (waveState_ == WaveState::Clear) {
		if (spriteClear_)
			spriteClear_->Draw();
	}

#ifdef _DEBUG
	ImGui::Begin("HUD");
	ImGui::Text("WAVE: %d", currentWave_);
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