#pragma once
#include "object.h"

// 정해야 하는 것
// 텍스처의 크기, 텍스트의 크기
// 텍스처의 크기는 Object에서 정함
// 텍스트의 크기는?

class UIObject : public Object
{
public:
	UIObject() = default;
	UIObject(sf::Vector2f position, sf::Vector2f size);
	virtual ~UIObject() = default;

	virtual void Update(float timeElapsed) override;
	virtual void Render(const shared_ptr<sf::RenderWindow>& window) override;

	virtual void OnProcessingKeyboardMessage(sf::Event inputEvent);
	virtual void OnProcessingMouseMessage(sf::Event inputEvent, const shared_ptr<sf::RenderWindow>& window);

	virtual void SetPosition(sf::Vector2f position) override;

	void SetChild(const shared_ptr<UIObject>& uiObject);

	void SetText(const char* text);
	void SetTextSize(int size);
	void SetTextColor(sf::Color color);
	void SetTextFont(sf::Font font);
	
	string GetString();

protected:
	BOOL							m_enable;
	vector<shared_ptr<UIObject>>	m_children;
	sf::Text						m_text;
	sf::Font						m_textFont;
};

class ButtonUIObject : public UIObject
{
public:
	enum class Type {
		NOACTIVE,
		MOUSEON,
		ACTIVE
	};

	ButtonUIObject() = default;
	ButtonUIObject(sf::Vector2f position, sf::Vector2f size);
	virtual ~ButtonUIObject() = default;

	virtual void Update(float timeElapsed) override;
	virtual void Render(const shared_ptr<sf::RenderWindow>& window) override;

	virtual void OnProcessingMouseMessage(sf::Event inputEvent, const shared_ptr<sf::RenderWindow>& window) override;

	virtual void SetClickEvent(function<void()> clickEvent);

protected:
	Type				m_type;	// is Selected?
	function<void()>	m_clickEvent;
};

constexpr int DELETE_KEY = 8;
constexpr int ENTER_KEY = 13;
constexpr int ESCAPE_KEY = 27;

class InputTextBoxUI : public ButtonUIObject
{
public:
	InputTextBoxUI() = default;
	InputTextBoxUI(sf::Vector2f position, sf::Vector2f size, INT limit);
	virtual ~InputTextBoxUI() = default;

	virtual void Update(float timeElapsed) override;
	virtual void Render(const shared_ptr<sf::RenderWindow>& window) override;

	virtual void OnProcessingKeyboardMessage(sf::Event inputEvent) override;
	virtual void OnProcessingMouseMessage(sf::Event inputEvent, const shared_ptr<sf::RenderWindow>& window) override;

	void SetTextLimit(INT limit);

private:
	void SetInputLogic(INT charType);
	void DeleteLastChar();

protected:
	ostringstream	m_texting;
	int				m_limit;

	const FLOAT		m_caretLifetime = 0.5f;
	FLOAT			m_caretTime;
	BOOL			m_caret;
};