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
	m_scene = make_unique<LoginScene>();

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
	m_scene->OnProcessingKeyboardMessage(inputEvent);
}

void GameFramework::OnProcessingMouseMessage(sf::Event inputEvent, const shared_ptr<sf::RenderWindow>& window)
{
	m_scene->OnProcessingMouseMessage(inputEvent, window);
}

void GameFramework::FrameAdvance()
{
	Timer::GetInstance().Tick();
	Update(Timer::GetInstance().GetDeltaTime());
	Render(g_window);
}

void GameFramework::Update(float timeElapsed)
{
	m_scene->Update(timeElapsed);
}

void GameFramework::Render(const shared_ptr<sf::RenderWindow>& window)
{
	m_scene->Render(window);
}

void GameFramework::ChangeScene(INT tag)
{
	switch (tag)
	{
	case Scene::Login:
		m_scene = make_unique<LoginScene>();
		break;
	case Scene::Main:
		m_scene = make_unique<MainScene>();
		break;
	}
}
