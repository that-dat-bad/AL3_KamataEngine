#pragma once
#include "TitleScene.h"

class StageSelectScene {
public:
	void Initialize();
	Scene Update();
	void Draw();

private:
	int selectedStage_ = 0;
};