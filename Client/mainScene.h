#pragma once
#include "scene.h"
#include "object.h"
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

	void AddPlayer(int id, sf::Vector2f position, const char* name);
	void ExitPlayer(int id);

	void Move(INT id, sf::Vector2f position);
	void SetChat(INT id, const char* chat);
	void SetAnimationInfo(int characterInfo, const shared_ptr<AnimationObject>& object);

private:
	virtual void BuildObjects() final;
	virtual void DestroyObject() final;

private:
	array<array<int, W_WIDTH>, W_HEIGHT> m_map;
	const float			m_moveTime = 0.01f;
	float				m_pressedMoveKey;
	shared_ptr<Object>	m_whiteTile;
	shared_ptr<Object>	m_blackTile;

	shared_ptr<Player>	m_avatar;
	unordered_map<INT, shared_ptr<Player>> m_players;
};