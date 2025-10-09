#pragma once
#include "KamataEngine.h"
#include <stdint.h>


class Player {
public:
	void Initialize(KamataEngine::Model* model,uint32_t textureHandle,KamataEngine::Camera* camera);

	void Update();

	void Draw();

private:

	KamataEngine::WorldTransform worldTransform_={};
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	uint32_t textureHandle_ = 0u;

};
