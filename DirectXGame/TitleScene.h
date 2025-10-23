#pragma once
#include "KamataEngine.h"
#include "SceneManager.h"

class TitleScene {
public:
	void Initialize();
	SceneManager Update();
	void Draw();

private:
	uint32_t titleTexture_ = 0;
	uint32_t textTexture_ = 0;
};