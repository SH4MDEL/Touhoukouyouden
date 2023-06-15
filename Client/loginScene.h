#pragma once
#include "scene.h"
#include "ui.h"

class LoginScene : public Scene
{
public:
	LoginScene();
	virtual ~LoginScene();

	void Update(float timeElapsed) final;
	void Render(const shared_ptr<sf::RenderWindow>& window) final;

	void OnProcessingKeyboardMessage(float timeElapsed) final;
	void OnProcessingInputTextMessage(sf::Event inputEvent) final;
	void OnProcessingMouseMessage(sf::Event inputEvent, const shared_ptr<sf::RenderWindow>& window) final;

	void ProcessPacket(char* buf) final;

private:
	virtual void BuildObjects() final;
	virtual void DestroyObject() final;

private:
	shared_ptr<ButtonUIObject>	m_gameStartUI;
	shared_ptr<InputTextBoxUI>	m_idBox;
	shared_ptr<InputTextBoxUI>	m_passwordBox;
};

