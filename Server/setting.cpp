#include "setting.h"

Setting::Setting()
{
	using namespace chrono;
	m_luaState = luaL_newstate();
	lua_gc(m_luaState, LUA_GCSTOP);

	luaL_openlibs(m_luaState);
	luaL_loadfile(m_luaState, "monster.lua");
	lua_pcall(m_luaState, 0, 0, 0);

	for (int i = Serial::Monster::START; i < Serial::Monster::COUNT; ++i) {
		string str = "MONSTER";
		lua_getglobal(m_luaState, (str + to_string(i - Serial::Monster::START) + "_NAME").c_str());
		lua_getglobal(m_luaState, (str + to_string(i - Serial::Monster::START) + "_LEVEL").c_str());
		lua_getglobal(m_luaState, (str + to_string(i - Serial::Monster::START) + "_EXP").c_str());
		lua_getglobal(m_luaState, (str + to_string(i - Serial::Monster::START) + "_HP").c_str());
		lua_getglobal(m_luaState, (str + to_string(i - Serial::Monster::START) + "_SPEED").c_str());
		lua_getglobal(m_luaState, (str + to_string(i - Serial::Monster::START) + "_ATK").c_str());
		lua_getglobal(m_luaState, (str + to_string(i - Serial::Monster::START) + "_MOVETYPE").c_str());
		lua_getglobal(m_luaState, (str + to_string(i - Serial::Monster::START) + "_WAITTYPE").c_str());

		m_monsterSetting.insert({ i, {} });

		m_monsterSetting[i].name = lua_tostring(m_luaState, 1);
		m_monsterSetting[i].level = lua_tointeger(m_luaState, 2);
		m_monsterSetting[i].exp = lua_tointeger(m_luaState, 3);
		m_monsterSetting[i].hp = lua_tointeger(m_luaState, 4);
		m_monsterSetting[i].speed = milliseconds(static_cast<int>(lua_tointeger(m_luaState, 5)));
		m_monsterSetting[i].atk = lua_tointeger(m_luaState, 6);
		m_monsterSetting[i].moveType = lua_tointeger(m_luaState, 7);
		m_monsterSetting[i].waitType = lua_tointeger(m_luaState, 8);

		lua_pop(m_luaState, 8);
	}
}

const MonsterSetting& Setting::GetMonsterSetting(int serial)
{
	return m_monsterSetting[serial];
}
