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

	void OnProcessingInputTextMessage(sf::Event inputEvent);
	void OnProcessingMouseMessage(sf::Event inputEvent, const shared_ptr<sf::RenderWindow>& window);

	void FrameAdvance();
	void Update(float timeElapsed);
	void Render(const shared_ptr<sf::RenderWindow>& window);

	void SetIsActive(bool isActive);

	Scene* GetScene() { return m_scene.get(); }
	void ChangeScene(INT tag);

private:
	bool					m_isActive;
	unique_ptr<Scene>		m_scene;
};

