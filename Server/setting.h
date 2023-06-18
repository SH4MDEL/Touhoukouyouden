#pragma once
#include "stdafx.h"
#include "singleton.h"

struct MonsterSetting
{
	MonsterSetting() = default;

	string name;
	int level;
	int exp;
	int hp;
	chrono::milliseconds speed;
	int atk;
	int waitType;
	int moveType;
};

struct CharacterSetting
{
	CharacterSetting() = default;

	int baseAtk;
	int bonusAtk;
	int baseSkill;
	int bonusSkill;
};

class Setting : public Singleton<Setting>
{
public:
	Setting();
	~Setting() = default;

	const MonsterSetting& GetMonsterSetting(int serial);
	const CharacterSetting& GetCharacterSetting(int serial);

private:
	concurrency::concurrent_unordered_map<int, MonsterSetting> m_monsterSetting;
	lua_State* m_monsterState;

	concurrency::concurrent_unordered_map<int, CharacterSetting> m_characterSetting;
	lua_State* m_characterState;
};

