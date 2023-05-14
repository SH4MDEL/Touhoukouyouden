#pragma once
#include "stdafx.h"
#include "timer.h"
#include "scene.h"

class Scene;
class GameFramework
{
public:
	GameFramework();
	~GameFramework() = default;

	void OnCreate();
	void BulidObject();

	void OnDestroy();
	void DestroyObject();

	void OnProcessingKeyboardMessage(sf::Event inputEvent);

	void FrameAdvance();
	void Update(float timeElapsed);
	void Render(const shared_ptr<sf::RenderWindow>& window);

	Scene* GetScene() { return m_scenes[m_sceneIndex].get(); }

private:
	array<unique_ptr<Scene>, 1>		m_scenes;
	INT								m_sceneIndex;
};

