#include "framework.h"

#include "loginScene.h"
#include "mainScene.h"

GameFramework::GameFramework()
{
}

void GameFramework::OnCreate()
{
	BulidObject();
}

void GameFramework::BulidObject()
{
	m_sceneIndex = Scene::Tag::Main;
	m_scenes[Scene::Tag::Login] = make_unique<LoginScene>();
	m_scenes[Scene::Tag::Main] = make_unique<MainScene>();

	m_scenes[m_sceneIndex]->OnCreate();

	Timer::GetInstance().Tick();
}

void GameFramework::OnDestroy()
{
}

void GameFramework::DestroyObject()
{
}

void GameFramework::OnProcessingKeyboardMessage(sf::Event inputEvent)
{
	m_scenes[m_sceneIndex]->OnProcessingKeyboardMessage(inputEvent);
}

void GameFramework::FrameAdvance()
{
	Timer::GetInstance().Tick();
	Update(Timer::GetInstance().GetDeltaTime());
	Render(g_window);
}

void GameFramework::Update(float timeElapsed)
{
	m_scenes[m_sceneIndex]->Update(timeElapsed);
}

void GameFramework::Render(const shared_ptr<sf::RenderWindow>& window)
{
	m_scenes[m_sceneIndex]->Render(window);
}
