#include "loginScene.h"

LoginScene::LoginScene()
{
	BuildObjects();
}

LoginScene::~LoginScene()
{
	DestroyObject();
}

void LoginScene::BuildObjects()
{
	m_buttonTexture = make_shared<sf::Texture>();
	m_buttonTexture->loadFromFile("Resource\\UI\\Button.png");

	m_gameStartUI = make_shared<ButtonUIObject>(sf::Vector2f{ 0, 0 }, sf::Vector2f{ 0.8f, 0.5f });
	m_gameStartUI->SetPosition(sf::Vector2f{ 100, 0 });
	m_gameStartUI->SetSpriteTexture(m_buttonTexture, 0, 0, 278, 115);
	m_gameStartUI->SetText("Game Start");
	m_gameStartUI->SetTextFont(g_font);
	m_gameStartUI->SetTextColor(sf::Color(255, 255, 255));
	m_gameStartUI->SetClickEvent([&]() {
		g_gameFramework.ChangeScene(Tag::Main);
	});

	m_idBox = make_shared<InputTextBoxUI>(sf::Vector2f{ 200.f, 100.f }, sf::Vector2f{ 2.f, 1.f }, 20);
	m_idBox->SetPosition(sf::Vector2f{ 200, 100 });
	m_idBox->SetSpriteTexture(m_buttonTexture, 0, 0, 278, 115);
	m_idBox->SetTextFont(g_font);
	m_idBox->SetTextColor(sf::Color(255, 255, 255));

	m_passwordBox = make_shared<InputTextBoxUI>(sf::Vector2f{ 200.f, 200.f }, sf::Vector2f{ 2.f, 1.f }, 20);
}

void LoginScene::DestroyObject()
{

}

void LoginScene::Update(float timeElapsed)
{
	m_idBox->Update(timeElapsed);
}

void LoginScene::Render(const shared_ptr<sf::RenderWindow>& window)
{
	m_gameStartUI->Render(window);
	m_idBox->Render(window);
	m_passwordBox->Render(window);
}

void LoginScene::OnProcessingKeyboardMessage(sf::Event inputEvent)
{
	switch (inputEvent.type)
	{
	case sf::Event::TextEntered:
	{
		m_idBox->OnProcessingKeyboardMessage(inputEvent);
		m_passwordBox->OnProcessingKeyboardMessage(inputEvent);
		break;
	}
	}
}

void LoginScene::OnProcessingMouseMessage(sf::Event inputEvent, const shared_ptr<sf::RenderWindow>& window)
{
	m_gameStartUI->OnProcessingMouseMessage(inputEvent, window);
	m_idBox->OnProcessingMouseMessage(inputEvent, window);
	m_passwordBox->OnProcessingMouseMessage(inputEvent, window);
}
