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

class Setting : public Singleton<Setting>
{
public:
	Setting();
	~Setting() = default;

	const MonsterSetting& GetMonsterSetting(int serial);

private:
	concurrency::concurrent_unordered_map<int, MonsterSetting> m_monsterSetting;
	lua_State* m_luaState;
};

