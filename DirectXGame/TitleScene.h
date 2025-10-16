#pragma once
#include "KamataEngine.h"

enum class Scene {
	kTitle,
	kStageSelect,
	kGame,
};

class TitleScene {
public:
	void Initialize();
	Scene Update();
	void Draw();

private:
	uint32_t titleTexture_ = 0;
	uint32_t promptTexture_ = 0;
};