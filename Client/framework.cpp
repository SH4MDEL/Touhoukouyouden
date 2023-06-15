#include "framework.h"

#include "loginScene.h"
#include "mainScene.h"

GameFramework::GameFramework() : m_isActive {true}
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

void GameFramework::OnProcessingInputTextMessage(sf::Event inputEvent)
{
	if (m_isActive) m_scene->OnProcessingInputTextMessage(inputEvent);
}

void GameFramework::OnProcessingMouseMessage(sf::Event inputEvent, const shared_ptr<sf::RenderWindow>& window)
{
	m_scene->OnProcessingMouseMessage(inputEvent, window);
}

void GameFramework::FrameAdvance()
{
	Timer::GetInstance().Tick();
	if (m_isActive) {
		m_scene->OnProcessingKeyboardMessage(Timer::GetInstance().GetDeltaTime());
	}
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

void GameFramework::SetIsActive(bool isActive)
{
	m_isActive = isActive;
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
