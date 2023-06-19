myid = 99999;

function set_uid(x)
	myid = x;
end

function event_player_move(player)
	player_x = API_GetX(player);
	player_y = API_GetY(player);
	my_x = API_GetX(myid);
	my_y = API_GetY(myid);
	if (player_x == my_x) then
		if (player_y == my_y) then
			API_SendMessage(myid, player, "HELLO");
		end
	end
end

-- 리턴값에 따른 이동
-- 0	dx = 0, dy = 1
-- 1	dx = 0, dy = -1
-- 2	dx = 1, dy = 0
-- 3	dx = -1, dy = 0
-- -1	길찾기 실패

function pathfinding(target)
	result = API_AStar(myid, target);
	return result
end