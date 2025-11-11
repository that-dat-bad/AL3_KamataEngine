// ResultScene.h
#pragma once
#include "IScene.h" // ★追加

// ISceneを継承する
class ResultScene : public IScene {
public:
	~ResultScene() override; // ★追加

	// 必要な関数をオーバーライド
	void Initialize() override;               // ★追加
	std::optional<SceneID> Update() override; // ★追加
	void Draw() override;                     // ★追加
};