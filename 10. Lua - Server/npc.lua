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

function event_player_leave(player)
	player_x = API_GetX(player);
	player_y = API_GetY(player);
	my_x = API_GetX(myid);
	my_y = API_GetY(myid);
	if (player_x == my_x) then
		if (player_y == my_y) then
			API_SendMessage(myid, player, "BYE");
		end
	end
end