#pragma once
#include "scene.h"
#include "object.h"
#include "ui.h"
#include "player.h"

class MainScene : public Scene
{
public:
	MainScene();
	virtual ~MainScene();

	void Update(float timeElapsed) final;
	void Render(const shared_ptr<sf::RenderWindow>& window) final;
	
	void OnProcessingKeyboardMessage(float timeElapsed) final;
	void OnProcessingInputTextMessage(sf::Event inputEvent) final;
	void OnProcessingMouseMessage(sf::Event inputEvent, const shared_ptr<sf::RenderWindow>& window) final;

	void AddPlayer(int id, int serial, sf::Vector2f position, const char* name);
	void ExitPlayer(int id);

	void Move(INT id, sf::Vector2f position);
	void SetChat(INT id, const char* chat);
	void SetAnimationInfo(int characterInfo, const shared_ptr<AnimationObject>& object);

	void LoginInfoProcess(char* buf);
	void AddObjectProcess(char* buf);
	void RemoveObjectProcess(char* buf);
	void MoveObjectProcess(char* buf);
	void ChatProcess(char* buf);
	void StatChangeProcess(char* buf);
	void ChangeHpProcess(char* buf);
	void DeadObjectProcess(char* buf);

private:
	virtual void BuildObjects() final;
	virtual void DestroyObject() final;

	void SetMessage(const char* message);

private:
	array<array<int, W_WIDTH>, W_HEIGHT> m_map;

	shared_ptr<UIObject>			m_levelUI;
	shared_ptr<UIObject>			m_hpUI;
	shared_ptr<UIObject>			m_expUI;
	array<shared_ptr<UIObject>, 20>	m_message;
	shared_ptr<InputTextBoxUI>		m_messageBox;

	shared_ptr<Object>				m_block;
	shared_ptr<Object>				m_nonblock;
	shared_ptr<Object>				m_henesysBlock;
	shared_ptr<Object>				m_henesysNonblock;

	shared_ptr<Player>	m_avatar;
	unordered_map<INT, shared_ptr<Player>> m_players;

	bool m_inputState;
	const float m_chatCoolTime = 1.f;
	float m_chatTime;
};