local function PlayerName(player)
	local puns = {
		--TODO: add moar...
		["asd"] = "qwe";
		["qwe"] = "asd";
	};
	
	return LoadFont("_arial black 20px")..{
		--Text="PlaceHolder";
		InitCommand=function(self)
			self:zoomx(1);
		end;
		SetCommand=function(self)
			self:settext("");
			if PROFILEMAN:IsPersistentProfile(player) then
				local profile = PROFILEMAN:GetProfile(player)
				local name = profile:GetDisplayName()
				
				if name == "" then
					name = string.gsub(player,"Number_P"," ")
				end
				--max 20 chars ºvº
				name = string.sub(name,1,20)
				--name = puns[name] or name
				
				self:settext(name)
			end
		end;
		PlayerJoinedMessageCommand=function(self)
			self:playcommand("Set");
		end;
		CoinInsertedMessageCommand=function(self)
			self:playcommand("Set");
		end;
		CoinModeChangedMessageCommand=function(self)
			self:playcommand("Set");
		end;
		ScreenChangedMessageCommand=function(self)
			self:playcommand("Set");
		end;
		StorageDevicesChangedMessageCommand=function(self)
			self:playcommand("Set");
		end;
	};
end

return Def.ActorFrame {
	LoadFont("_fixedsys")..{
		InitCommand=function(self)
			self:blend('BlendMode_Add');
			self:xy(10, 5);
			self:shadowlength(0);
			self:Stroke(0, 0, 0, 1);
			self:wrapwidthpixels(SCREEN_WIDTH * 0.45);
			self:maxwidth(SCREEN_WIDTH * 0.45);
			self:horizalign(left);
			self:SetTextureFiltering(false);
			self:vertalign(top);
			self:zoom(1.5);
		end;
		SystemMessageMessageCommand=function(self,params)
			self:settext(params.Message);
			self:finishtweening();
			self:diffusealpha(1);
			self:sleep(3);
			self:diffusealpha(0);
		end;
		HideSystemMessageMessageCommand=function(self)
			self:finishtweening();
		end;
	};
	
	LoadFont("_arial black 20px")..{
		InitCommand=function(self)
			self:xy(SCREEN_CENTER_X, SCREEN_HEIGHT - 20);
			self:zoom(1);
			self:shadowlength(0);
			self:playcommand("Text");
		end;
		TextCommand=function(self)
			if GAMESTATE:GetCoinMode() == 'CoinMode_Home' then
				self:settext("Home Mode");
			else
				self:settext("Free Play")
			end
		end;
		PlayerJoinedMessageCommand=function(self)
			self:playcommand("Text");
		end;
		CoinInsertedMessageCommand=function(self)
			self:playcommand("Text");
		end;
		CoinModeChangedMessageCommand=function(self)
			self:playcommand("Text");
		end;
		ScreenChangedMessageCommand=function(self)
			self:playcommand("Text");
		end;
	};
	
	ScreenChangedMessageCommand=function(self)
		self:visible(THEME:GetMetric(SCREENMAN:GetTopScreen():GetName(),"ShowCreditDisplay"));
	end;
	
	PlayerName(PLAYER_1)..{
		InitCommand=function(self)
			self:xy(20, SCREEN_HEIGHT - 20);
			self:horizalign(left);
		end;
	};
	PlayerName(PLAYER_2)..{
		InitCommand=function(self)
			self:xy(SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20);
			self:horizalign(right);
		end;
	};
}
