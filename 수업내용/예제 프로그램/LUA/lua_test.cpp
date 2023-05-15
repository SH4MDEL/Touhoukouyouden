#include <iostream>
#include "include/lua.hpp"

#pragma comment (lib, "lua54.lib")
using namespace std;

int add_from_lua(lua_State* L)
{
	int a = lua_tonumber(L, -2);
	int b = lua_tonumber(L, -1);
	lua_pop(L, 3);
	int result = a + b;
	lua_pushnumber(L, result);
	return 1;
}

int main()
{
	const char* buff = "printt \"Hello from Lua.\"\n";

	lua_State* L = luaL_newstate();	// Lua 가상 머신을 만든다.
	luaL_openlibs(L);	// 기본 함수를 사용하기 위해 Open Library 로딩.

	//luaL_loadbuffer(L, buff, strlen(buff), "line");	// 가상 머신에 buff에 있는 문자열 로딩
	luaL_loadfile(L, "dragon_ai.lua");

	int error = lua_pcall(L, 0, 0, 0);	// lua_pcall을 통해 실행한다.
	if (error) {
		cout << "Error:" << lua_tostring(L, -1);
		lua_pop(L, 1);
	}
	lua_getglobal(L, "pos_x");
	lua_getglobal(L, "pos_y");
	int pos_x = lua_tonumber(L, -2);	// stack
	int pos_y = lua_tonumber(L, -1);
	lua_pop(L, 2);

	cout << "pos = (" << pos_x << ", " << pos_y << ")\n";

	lua_getglobal(L, "plustwo");
	lua_pushnumber(L, 100);
	lua_pcall(L, 1, 1, 0);
	int result = lua_tonumber(L, -1);
	lua_pop(L, 1);

	cout << "result : " << result << endl;

	lua_register(L, "c_add", add_from_lua);

	lua_getglobal(L, "lua_add");
	lua_pushnumber(L, 100);
	lua_pushnumber(L, 200);
	lua_pcall(L, 2, 1, 0);

	int sum = lua_tonumber(L, -1);
	lua_pop(L, 1);

	cout << "sum : " << sum << endl;

	lua_close(L);	// newstate를 통해 메모리를 할당받았으므로 해제한다.
}