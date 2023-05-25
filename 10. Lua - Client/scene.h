#pragma once
#include "stdafx.h"
#include "main.h"

class Scene abstract
{
public:
	enum Tag {
		Main,
		Count
	};

	Scene() = default;
	Scene(Tag tag);
	virtual ~Scene() = default;

	virtual void OnCreate() = 0;
	virtual void OnDestroy() = 0;

	virtual void Update(float timeElapsed) = 0;
	virtual void Render(const shared_ptr<sf::RenderWindow>& window) = 0;

	virtual void OnProcessingKeyboardMessage(sf::Event inputEvent) = 0;

	virtual void AddPlayer(int id, Short2 position, const char* name) = 0;
	virtual void ExitPlayer(int id) = 0;

protected:
	virtual void BuildObjects() = 0;
	virtual void DestroyObject() = 0;

protected:
	Tag		m_tag;
};

