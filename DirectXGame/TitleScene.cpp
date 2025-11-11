#include "TitleScene.h"
#include "KamataEngine.h"
#include <assert.h>

using namespace KamataEngine;

TitleScene::~TitleScene() {
	delete model_;
	delete debugCamera_;

}

void TitleScene::Initialize() {
	textureHandle_ = TextureManager::Load("UVChecker.png");
	model_ = Model::Create();
	worldTransform_.Initialize();
	camera_.Initialize();

	// Input繧､繝ｳ繧ｹ繧ｿ繝ｳ繧ｹ縺ｮ蜿門ｾ・
	input_ = Input::GetInstance();

	// 繝・ヰ繝・げ繧ｫ繝｡繝ｩ縺ｮ逕滓・
	debugCamera_ = new DebugCamera(1280, 720);

	// 霆ｸ譁ｹ蜷題｡ｨ遉ｺ縺ｮ陦ｨ遉ｺ繧呈怏蜉ｹ縺ｫ縺吶ｋ
	AxisIndicator::GetInstance()->SetVisible(true);
	// 霆ｸ譁ｹ蜷題｡ｨ遉ｺ縺悟盾辣ｧ縺吶ｋ繝薙Η繝ｼ繝励Ο繧ｸ繧ｧ繧ｯ繧ｷ繝ｧ繝ｳ繧呈欠螳壹☆繧・
	AxisIndicator::GetInstance()->SetTargetCamera(&camera_);

}

std::optional<SceneID> TitleScene::Update() {

#ifdef _DEBUG

	if (input_->TriggerKey(DIK_0)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}

	// デバッグカメラの状態表示
	if (isDebugCameraActive_) {
		ImGui::Begin("Debug Camera");
		ImGui::Text("Debug Camera: ON");
		ImGui::End();
	}

#endif

	// カメラの更新
	if (isDebugCameraActive_) {
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		camera_.TransferMatrix();
	} else {
		camera_.UpdateMatrix();
		camera_.TransferMatrix();
	}
	return std::nullopt; // 戻り値漏れ防止
}

void TitleScene::Draw() {

	KamataEngine::DirectXCommon* dxCommon = KamataEngine::DirectXCommon::GetInstance();

	// 繝｢繝・Ν縺ｮ謠冗判貅門ｙ
	KamataEngine::Model::PreDraw(dxCommon->GetCommandList());


	// 繝｢繝・Ν縺ｮ謠冗判邨ゆｺ・
	KamataEngine::Model::PostDraw();

	// 霆ｸ譁ｹ蜷題｡ｨ遉ｺ縺ｮ謠冗判
	AxisIndicator::GetInstance()->Draw();
}