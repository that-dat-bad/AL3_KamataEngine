#pragma once
#include "IScene.h"

class GameScene;
class TitleScene;

class SceneManager {
public:

	SceneManager();
	~SceneManager(); 

	void Initialize(); // 最初のシーンを生成・初期化
	void Update();     // 現在のシーンの更新
	void Draw();       // 現在のシーンの描画

	void ChangeScene(SceneID nextSceneID); // シーン変更をリクエスト

private:
	IScene* currentScene_ = nullptr; 
	SceneID currentSceneID_ = SceneID::kTitle; // 現在のシーンの種類
};