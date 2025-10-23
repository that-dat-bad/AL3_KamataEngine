#pragma once
#include "KamataEngine.h"
#include "SceneManager.h"

class ResultScene {
public:
	void Initialize();
	SceneManager Update();
	void Draw();

private:
	uint32_t resultTexture_ = 0;
	uint32_t textTexture_ = 0;
};