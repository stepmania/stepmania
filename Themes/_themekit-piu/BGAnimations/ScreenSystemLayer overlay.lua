local function PlayerName(player)
	local puns = {
		--TODO: add moar...
		["asd"] = "qwe";
		["qwe"] = "asd";
	};
	
	return LoadFont("_arial black 20px")..{
		--Text="PlaceHolder";
		InitCommand=cmd(zoomx,1);
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
		PlayerJoinedMessageCommand=cmd(playcommand,"Set");
		CoinInsertedMessageCommand=cmd(playcommand,"Set");
		CoinModeChangedMessageCommand=cmd(playcommand,"Set");
		ScreenChangedMessageCommand=cmd(playcommand,"Set");
		StorageDevicesChangedMessageCommand=cmd(playcommand,"Set");
	};
end

return Def.ActorFrame {
	LoadFont("_fixedsys")..{
		InitCommand=cmd(blend,'BlendMode_Add'xy,10,5;shadowlength,0;Stroke,{0,0,0,1};wrapwidthpixels,SCREEN_WIDTH*0.45;maxwidth,SCREEN_WIDTH*0.45;horizalign,left;SetTextureFiltering,false;vertalign,top;zoom,1.5);
		SystemMessageMessageCommand=function(self,params)
			self:settext(params.Message);
			self:finishtweening();
			self:diffusealpha(1);
			self:sleep(3);
			self:diffusealpha(0);
		end;
		HideSystemMessageMessageCommand=cmd(finishtweening);
	};
	
	LoadFont("_arial black 20px")..{
		InitCommand=cmd(xy,SCREEN_CENTER_X,SCREEN_HEIGHT-20;zoom,1;shadowlength,0;playcommand,"Text");
		TextCommand=function(self)
			if GAMESTATE:GetCoinMode() == 'CoinMode_Pay' then
				local Coins = GAMESTATE:GetCoins()
				local CoinForCredit = GAMESTATE:GetCoinsNeededToJoin()
				local Credits = math.floor(Coins/CoinForCredit)
				local Remain = Coins % CoinForCredit
				self:settext(string.format("CREDIT(S) %i[%i/%i]", Credits, Remain, CoinForCredit))
			elseif GAMESTATE:GetCoinMode() == 'CoinMode_Free' then
				self:settext("Free Play")
			else --homemode
				self:settext("Home Mode");
			end
		end;
		PlayerJoinedMessageCommand=cmd(playcommand,"Text");
		CoinInsertedMessageCommand=cmd(playcommand,"Text");
		CoinModeChangedMessageCommand=cmd(playcommand,"Text");
		ScreenChangedMessageCommand=cmd(playcommand,"Text");
	};
	
	ScreenChangedMessageCommand=function(self)
		self:visible(THEME:GetMetric(SCREENMAN:GetTopScreen():GetName(),"ShowCreditDisplay"));
	end;
	
	PlayerName(PLAYER_1)..{
		InitCommand=cmd(xy,20,SCREEN_HEIGHT-20;horizalign,left);
	};
	PlayerName(PLAYER_2)..{
		InitCommand=cmd(xy,SCREEN_WIDTH-20,SCREEN_HEIGHT-20;horizalign,right);
	};
}
