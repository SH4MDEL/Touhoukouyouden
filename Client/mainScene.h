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

	void ProcessPacket(char* buf) final;

	void AddPlayer(int id, int serial, sf::Vector2f position, const char* name);
	void ExitPlayer(int id);

	void Move(INT id, sf::Vector2f position);
	void SetChat(INT id, const char* chat);
	void SetAnimationInfo(int characterInfo, const shared_ptr<AnimationObject>& object);

private:
	virtual void BuildObjects() final;
	virtual void DestroyObject() final;

private:
	array<array<int, W_WIDTH>, W_HEIGHT> m_map;

	shared_ptr<UIObject>	m_levelUI;
	shared_ptr<UIObject>	m_hpUI;
	shared_ptr<UIObject>	m_expUI;

	shared_ptr<Object>	m_block;
	shared_ptr<Object>	m_nonblock;
	shared_ptr<Object>	m_henesysBlock;
	shared_ptr<Object>	m_henesysNonblock;

	shared_ptr<Player>	m_avatar;
	unordered_map<INT, shared_ptr<Player>> m_players;
};