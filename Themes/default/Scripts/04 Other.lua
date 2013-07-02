function SongMeterDisplayX(pn)
	if Center1Player() then
		return SCREEN_CENTER_X
	else
		return pn == PLAYER_1 and SCREEN_LEFT+16 or SCREEN_RIGHT-16
	end
end

function SongMeterDisplayY(pn)
	return Center1Player() and SCREEN_TOP+50 or SCREEN_CENTER_Y
end

function SongMeterDisplayCommand(pn)
	if Center1Player() then
		return function(self)
			self:draworder(50);
			self:zoom(0);
			self:y(SCREEN_TOP - 24);
			self:sleep(1.5);
			self:decelerate(0.5);
			self:zoom(1);
			self:y(SCREEN_TOP + 50);
		end;
	else
		local xAdd = (pn == PLAYER_1) and -24 or 24;
		return function(self)
			self:draworder(5);
			self:rotationz(-90);
			self:zoom(0);
			self:addx(xAdd);
			self:sleep(1.5);
			self:decelerate(0.5);
			self:zoom(1);
			self:addx(xAdd * -1);
		end;
	end
end