#pragma once
#include "scene.h"

class LoginScene : public Scene
{
public:
	LoginScene() = default;
	LoginScene(Tag tag);
	virtual ~LoginScene() = default;

	virtual void OnCreate() override;
	virtual void OnDestroy() override;

	void Update(float timeElapsed) override;
	void Render(const shared_ptr<sf::RenderWindow>& window) override;

	void OnProcessingKeyboardMessage(sf::Event inputEvent) override;

private:
	virtual void BuildObjects() override;
	virtual void DestroyObject() override;

private:

};

