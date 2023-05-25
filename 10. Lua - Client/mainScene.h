#pragma once
#include "scene.h"
#include "object.h"
#include "piece.h"

class MainScene : public Scene
{
public:
	MainScene() = default;
	MainScene(Tag tag);
	virtual ~MainScene() = default;

	virtual void OnCreate() override;
	virtual void OnDestroy() override;

	void Update(float timeElapsed) override;
	void Render(const shared_ptr<sf::RenderWindow>& window) override;
	
	void OnProcessingKeyboardMessage(sf::Event inputEvent) override;

	void AddPlayer(int id, Short2 position, const char* name) override;
	void ExitPlayer(int id) override;

	void Move(INT id, Short2 position);
	void SetChat(INT id, const char* chat);

private:
	virtual void BuildObjects() override;
	virtual void DestroyObject() override;

private:
	shared_ptr<sf::Texture>	m_boardTexture;
	shared_ptr<sf::Texture>	m_pieceTexture;

	shared_ptr<Object> m_whiteTile;
	shared_ptr<Object> m_blackTile;

	shared_ptr<Piece> m_avatar;
	unordered_map<INT, shared_ptr<Piece>> m_players;
};