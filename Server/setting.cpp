#include "setting.h"

Setting::Setting()
{
	using namespace chrono;

	m_characterState = luaL_newstate();
	lua_gc(m_characterState, LUA_GCSTOP);

	luaL_openlibs(m_characterState);
	luaL_loadfile(m_characterState, "character.lua");
	lua_pcall(m_characterState, 0, 0, 0);

	for (int i = Serial::Character::START; i < Serial::Character::COUNT; ++i) {
		string str = "CHAR";
		lua_getglobal(m_characterState, (str + to_string(i - Serial::Character::START) + "_BASE_ATK").c_str());
		lua_getglobal(m_characterState, (str + to_string(i - Serial::Character::START) + "_BONUS_ATK").c_str());
		lua_getglobal(m_characterState, (str + to_string(i - Serial::Character::START) + "_BASE_SKILL").c_str());
		lua_getglobal(m_characterState, (str + to_string(i - Serial::Character::START) + "_BONUS_SKILL").c_str());

		m_characterSetting.insert({ i, {} });

		m_characterSetting[i].baseAtk = lua_tointeger(m_characterState, 1);
		m_characterSetting[i].bonusAtk = lua_tointeger(m_characterState, 2);
		m_characterSetting[i].baseSkill = lua_tointeger(m_characterState, 3);
		m_characterSetting[i].bonusSkill = lua_tointeger(m_characterState, 4);

		lua_pop(m_characterState, 4);
	}

	m_monsterState = luaL_newstate();
	lua_gc(m_monsterState, LUA_GCSTOP);

	luaL_openlibs(m_monsterState);
	luaL_loadfile(m_monsterState, "monster.lua");
	lua_pcall(m_monsterState, 0, 0, 0);

	for (int i = Serial::Monster::START; i < Serial::Monster::COUNT; ++i) {
		string str = "MONSTER";
		lua_getglobal(m_monsterState, (str + to_string(i - Serial::Monster::START) + "_NAME").c_str());
		lua_getglobal(m_monsterState, (str + to_string(i - Serial::Monster::START) + "_LEVEL").c_str());
		lua_getglobal(m_monsterState, (str + to_string(i - Serial::Monster::START) + "_EXP").c_str());
		lua_getglobal(m_monsterState, (str + to_string(i - Serial::Monster::START) + "_HP").c_str());
		lua_getglobal(m_monsterState, (str + to_string(i - Serial::Monster::START) + "_SPEED").c_str());
		lua_getglobal(m_monsterState, (str + to_string(i - Serial::Monster::START) + "_ATK").c_str());
		lua_getglobal(m_monsterState, (str + to_string(i - Serial::Monster::START) + "_MOVETYPE").c_str());
		lua_getglobal(m_monsterState, (str + to_string(i - Serial::Monster::START) + "_WAITTYPE").c_str());

		m_monsterSetting.insert({ i, {} });

		m_monsterSetting[i].name = lua_tostring(m_monsterState, 1);
		m_monsterSetting[i].level = lua_tointeger(m_monsterState, 2);
		m_monsterSetting[i].exp = lua_tointeger(m_monsterState, 3);
		m_monsterSetting[i].hp = lua_tointeger(m_monsterState, 4);
		m_monsterSetting[i].speed = milliseconds(static_cast<int>(lua_tointeger(m_monsterState, 5)));
		m_monsterSetting[i].atk = lua_tointeger(m_monsterState, 6);
		m_monsterSetting[i].moveType = lua_tointeger(m_monsterState, 7);
		m_monsterSetting[i].waitType = lua_tointeger(m_monsterState, 8);

		lua_pop(m_monsterState, 8);
	}
}

const CharacterSetting& Setting::GetCharacterSetting(int serial)
{
	return m_characterSetting[serial];
}

const MonsterSetting& Setting::GetMonsterSetting(int serial)
{
	return m_monsterSetting[serial];
}
