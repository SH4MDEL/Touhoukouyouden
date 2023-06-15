#pragma once
#include "stdafx.h"
#include "main.h"

class Scene abstract
{
public:
	enum Tag {
		Login,
		Main,
		Count
	};

	Scene() = default;
	virtual ~Scene() = default;

	virtual void Update(float timeElapsed) = 0;
	virtual void Render(const shared_ptr<sf::RenderWindow>& window) = 0;

	virtual void OnProcessingKeyboardMessage(float timeElapsed) = 0;
	virtual void OnProcessingInputTextMessage(sf::Event inputEvent) = 0;
	virtual void OnProcessingMouseMessage(sf::Event inputEvent, const shared_ptr<sf::RenderWindow>& window) = 0;

	void Recv();
	void TranslatePacket(char* buf, size_t io_byte);
	virtual void ProcessPacket(char* buf) = 0;

protected:
	virtual void BuildObjects() = 0;
	virtual void DestroyObject() = 0;

protected:
	Tag		m_tag;

public:
	unordered_map<string, shared_ptr<sf::Texture>> g_textures;
};

