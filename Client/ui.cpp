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
	float xPos = m_position.x + (m_sprite.getLocalBounds().width * m_size.x / 2) - (m_text.getLocalBounds().width / 2);
	float yPos = m_position.y + (m_sprite.getLocalBounds().height * m_size.y / 2) - (m_text.getLocalBounds().height);
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

	SetPosition(m_position);

	// ���¿� ���� �ٸ� ������ ������
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

	float buttonWidth = buttonXPos + m_sprite.getLocalBounds().width * m_size.x;
	float buttonHeigth = buttonYPos + m_sprite.getLocalBounds().height * m_size.y;

	if (mouseX < buttonWidth && mouseX > buttonXPos && mouseY < buttonHeigth && mouseY > buttonYPos) {
		// ���콺�� ��ģ ���¿��� Ŭ�� �߻�
		if (m_type == Type::MOUSEON && inputEvent.type == sf::Event::MouseButtonPressed) {
			m_type = Type::ACTIVE;
			g_clickEvent = m_clickEvent;
			m_sprite.setColor(sf::Color(145, 145, 145));
		}
		// �������� Ŭ���� �̹߻�
		else {
			m_type = Type::MOUSEON;
			m_sprite.setColor(sf::Color(200, 200, 200));
		}
	}
	else {
		// ���콺�� �ٸ� ��ġ�� ����.
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

	// �ؽ�Ʈ �ٰ� Ȱ��ȭ �� �����ϰ�� ĳ���� ���� ������Ʈ ����
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

	SetPosition(m_position);

	window->draw(m_sprite);
	// ��ġ ����
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

	float buttonWidth = buttonXPos + m_sprite.getLocalBounds().width * m_size.x;
	float buttonHeigth = buttonYPos + m_sprite.getLocalBounds().height * m_size.y;

	if (inputEvent.type == sf::Event::MouseButtonPressed) {
		// Ŭ�� �߻� �� ���콺�� ��ư ���� ����
		if (mouseX < buttonWidth && mouseX > buttonXPos && mouseY < buttonHeigth && mouseY > buttonYPos) {
			m_type = Type::ACTIVE;

			string oldText = m_texting.str();
			string newText = "";
			for (int i = 0; i < oldText.length(); ++i) {
				newText += oldText[i];
			}
			m_text.setString(m_texting.str());
		}
		// ���콺�� ��ư ���� ����
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

string InputTextBoxUI::GetString()
{
	return m_texting.str();
}

void InputTextBoxUI::SetInputLogic(INT charType)
{
	if (charType == DELETE_KEY) {
		if (m_texting.str().length() > 0) {
			DeleteLastChar();
		}
	}
	else if (charType == ENTER_KEY) {
		// �Է� ó��
		m_type = Type::NOACTIVE;
		m_caret = false;
		m_caretTime = 0.f;
	}
	else if (charType == ESCAPE_KEY) {
		// Ż��
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
