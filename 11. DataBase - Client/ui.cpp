#include "ui.h"

UIObject::UIObject(sf::Vector2f position, sf::Vector2f size) : Object(position, size), m_enable{true}
{
}

void UIObject::Update(float timeElapsed)
{
	if (!m_enable) return;

	for (auto& child : m_children) child->Update(timeElapsed);
}

void UIObject::Render(const shared_ptr<sf::RenderWindow>& window)
{
	Object::Render(window);

	window->draw(m_text);

	for (auto& child : m_children) child->Render(window);
}

void UIObject::OnProcessingKeyboardMessage(sf::Event inputEvent)
{
	if (!m_enable) return;
	for (auto& child : m_children) child->OnProcessingKeyboardMessage(inputEvent);
}

void UIObject::OnProcessingMouseMessage(sf::Event inputEvent, const shared_ptr<sf::RenderWindow>& window)
{
	if (!m_enable) return;
	for (auto& child : m_children) child->OnProcessingMouseMessage(inputEvent, window);
}

void UIObject::SetPosition(sf::Vector2f position)
{
	Object::SetPosition(position);
	float xPos = m_spritePosition.x + (m_sprite.getLocalBounds().width * m_spriteSize.x / 2) - (m_text.getLocalBounds().width / 2);
	float yPos = m_spritePosition.y + (m_sprite.getLocalBounds().height * m_spriteSize.y / 2) - (m_text.getLocalBounds().height);
	m_text.setPosition(xPos, yPos);
}

void UIObject::SetChild(const shared_ptr<UIObject>& uiObject)
{
	m_children.push_back(uiObject);
}

void UIObject::SetText(const char* text)
{
	m_text.setString(text);
}

void UIObject::SetTextSize(int size)
{
	m_text.setCharacterSize(size);
}

void UIObject::SetTextColor(sf::Color color)
{
	m_text.setFillColor(color);
}

void UIObject::SetTextFont(sf::Font font)
{
	m_textFont = font;
}

string UIObject::GetString()
{
	return m_text.getString().toAnsiString();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ButtonUIObject::ButtonUIObject(sf::Vector2f position, sf::Vector2f size) : UIObject(position, size), m_type{Type::NOACTIVE}
{
}

void ButtonUIObject::Update(float timeElapsed)
{
	if (!m_enable) return;

	for (auto& child : m_children) child->Update(timeElapsed);
}

void ButtonUIObject::Render(const shared_ptr<sf::RenderWindow>& window)
{
	if (!m_enable) return;

	SetPosition(m_spritePosition);

	// 상태에 따라 다른 색으로 렌더링
	if (m_type == Type::ACTIVE) {
		window->draw(m_sprite);
	}
	else if (m_type == Type::MOUSEON) {
		window->draw(m_sprite);
	}
	else {
		window->draw(m_sprite);
	}

	m_text.setFont(m_textFont);
	window->draw(m_text);

	for (auto& child : m_children) child->Render(window);
}

void ButtonUIObject::OnProcessingMouseMessage(sf::Event inputEvent, const shared_ptr<sf::RenderWindow>& window)
{
	if (!m_enable) return;

	float mouseX = sf::Mouse::getPosition(*window).x;
	float mouseY = sf::Mouse::getPosition(*window).y;

	float buttonXPos = m_sprite.getPosition().x;
	float buttonYPos = m_sprite.getPosition().y;

	float buttonWidth = buttonXPos + m_sprite.getLocalBounds().width * m_spriteSize.x;
	float buttonHeigth = buttonYPos + m_sprite.getLocalBounds().height * m_spriteSize.y;

	if (mouseX < buttonWidth && mouseX > buttonXPos && mouseY < buttonHeigth && mouseY > buttonYPos) {
		// 마우스와 겹친 상태에서 클릭 발생
		if (m_type == Type::MOUSEON && inputEvent.type == sf::Event::MouseButtonPressed) {
			m_type = Type::ACTIVE;
			//g_clickEvent = m_clickEvent;
			m_sprite.setColor(sf::Color(145, 145, 145));
		}
		// 겹쳤지만 클릭은 미발생
		else {
			m_type = Type::MOUSEON;
			m_sprite.setColor(sf::Color(200, 200, 200));
		}
	}
	else {
		// 마우스가 다른 위치에 있음.
		m_type = Type::NOACTIVE;
		m_sprite.setColor(sf::Color(255, 255, 255));
	}
}

void ButtonUIObject::SetClickEvent(function<void()> clickEvent)
{
	m_clickEvent = clickEvent;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

InputTextBoxUI::InputTextBoxUI(sf::Vector2f position, sf::Vector2f size, INT limit) : 
	ButtonUIObject(position, size), m_limit{ limit }, m_caretTime{ 0.f }, m_caret{ false }
{

}

void InputTextBoxUI::Update(float timeElapsed)
{
	if (!m_enable) return;

	// 텍스트 바가 활성화 된 상태일경우 캐럿에 관한 업데이트 진행
	if (m_type == Type::ACTIVE) {
		m_caretTime += timeElapsed;
		if (m_caretTime >= m_caretLifetime) {
			m_caretTime -= m_caretLifetime;
			m_caret = !m_caret;
		}
	}
	if (m_caret) {
		m_text.setString(m_texting.str() + "|");

	}
	else {
		m_text.setString(m_texting.str());
	}

	for (auto& child : m_children) child->Update(timeElapsed);
}

void InputTextBoxUI::Render(const shared_ptr<sf::RenderWindow>& window)
{
	if (!m_enable) return;

	SetPosition(m_spritePosition);

	window->draw(m_sprite);
	// 위치 지정
	m_text.setFont(m_textFont);
	window->draw(m_text);
}

void InputTextBoxUI::OnProcessingKeyboardMessage(sf::Event inputEvent)
{
	if (m_type == Type::ACTIVE) {
		int charType = inputEvent.text.unicode;
		if (charType < 128) {
			if (m_texting.str().length() < m_limit) {
				SetInputLogic(charType);
			}
			else if (m_texting.str().length() >= m_limit && charType == DELETE_KEY) {
				DeleteLastChar();
			}
		}
	}
}

void InputTextBoxUI::OnProcessingMouseMessage(sf::Event inputEvent, const shared_ptr<sf::RenderWindow>& window)
{
	if (!m_enable) return;

	float mouseX = sf::Mouse::getPosition(*window).x;
	float mouseY = sf::Mouse::getPosition(*window).y;

	float buttonXPos = m_sprite.getPosition().x;
	float buttonYPos = m_sprite.getPosition().y;

	float buttonWidth = buttonXPos + m_sprite.getLocalBounds().width * m_spriteSize.x;
	float buttonHeigth = buttonYPos + m_sprite.getLocalBounds().height * m_spriteSize.y;

	if (inputEvent.type == sf::Event::MouseButtonPressed) {
		// 클릭 발생 시 마우스가 버튼 위에 있음
		if (mouseX < buttonWidth && mouseX > buttonXPos && mouseY < buttonHeigth && mouseY > buttonYPos) {
			m_type = Type::ACTIVE;

			string oldText = m_texting.str();
			string newText = "";
			for (int i = 0; i < oldText.length(); ++i) {
				newText += oldText[i];
			}
			m_text.setString(m_texting.str());
		}
		// 마우스가 버튼 위에 없음
		else {
			m_type = Type::NOACTIVE;
			m_caret = false;
			m_caretTime = 0.f;
		}
	}
}

void InputTextBoxUI::SetTextLimit(INT limit)
{
	m_limit = limit;
}

void InputTextBoxUI::SetInputLogic(INT charType)
{
	if (charType == DELETE_KEY) {
		if (m_texting.str().length() > 0) {
			DeleteLastChar();
		}
	}
	else if (charType == ENTER_KEY) {
		// 입력 처리
		m_type = Type::NOACTIVE;
		m_caret = false;
		m_caretTime = 0.f;
	}
	else if (charType == ESCAPE_KEY) {
		// 탈출
		m_type = Type::NOACTIVE;
		m_caret = false;
		m_caretTime = 0.f;
	}
	else {
		m_texting << static_cast<char>(charType);
	}
	m_text.setString(m_texting.str());
}

void InputTextBoxUI::DeleteLastChar()
{
	string oldText = m_texting.str();
	string newText = "";
	for (int i = 0; i < oldText.length() - 1; ++i) {
		newText += oldText[i];
	}
	m_texting.str("");
	m_texting << newText;

	m_text.setString(m_texting.str());
}
