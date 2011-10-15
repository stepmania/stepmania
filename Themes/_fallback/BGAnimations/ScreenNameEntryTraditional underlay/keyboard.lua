local function distance( x1, y1, x2, y2 )
	return math.pow( math.pow(x1-x2, 2) + math.pow(y1-y2, 2), 0.5 );
end

local children = {
};

local Letters = {
	"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N",
	"O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "?", "!",
	"BACK", "ENTER"
};

local LetterIndexes = { };
for i, l in ipairs(Letters) do
	LetterIndexes[l] = i;
end

local MapNameToLetter = {};
local MapLetterToName = {};
local letter = LoadFont("Common Normal")..{
	InitCommand=cmd(zoom,0.8;shadowlength,0);
	PulseCommand = function(self)
		self:finishtweening();
		local z = self:GetZoomX();
		(cmd(accelerate,0.15;zoom,1.2;decelerate,0.15;zoom,z))(self);
	end;
};

for l in ivalues(Letters) do
	local Name = "letter " .. l;
	MapNameToLetter[Name] = l;
	MapLetterToName[l] = Name;
	children[#children+1] = letter..{
		Name = Name;
		Text = l;
	};
	if l == "BACK" or l == "ENTER" then
		children[#children].File = THEME:GetPathF("Common", "Normal");
	end;
end;
children[#children+1] = LoadActor( THEME:GetPathS("Common", "value") ) .. { Name = "Type"; SupportPan = true; }
children[#children+1] = LoadActor( THEME:GetPathS("Common", "cancel") ) .. { Name = "Back"; SupportPan = true; }
children[#children+1] = LoadActor( THEME:GetPathS("Common", "start") ).. { Name = "Enter"; SupportPan = true; }
children[#children+1] = LoadActor( THEME:GetPathS("MusicWheel", "change") ) .. { Name = "Move"; SupportPan = true; }

local CursorFiles = {
	[PLAYER_1] = "Cursor P1",
	[PLAYER_2] = "Cursor P2"
};

for pn in ivalues(PlayerNumber) do
	children[#children+1] = LoadActor( THEME:GetPathG("_frame", "1D"),
		{ 4/10, 2/10, 4/10 },
		LoadActor(CursorFiles[pn])
	)..{
		Name = CursorFiles[pn];
		BeginCommand = cmd(visible,SCREENMAN:GetTopScreen():GetEnteringName(pn));
		OnCommand = cmd(
				zoom,0;
				rotationz,-360*2;
				sleep,0.55;
				decelerate,0.5;
				zoom,1;
				rotationz,0;
			);
		OffCommand = cmd(sleep,0.3;queuecommand,"TweenOff");
		PlayerFinishedMessageCommand = function(self,param)
			if param.PlayerNumber ~= pn then return end
			self:playcommand("TweenOff" );
		end;
		TweenOffCommand = function(self,param)
			(cmd(accelerate,0.25;zoomx,0;))(self);
		end;
	};
end

local c;
local Keys = {};
local Selection = {};
local PlayerX = {};
local PlayerY = {};
return Def.ActorFrame {
	children = children;
	BeginCommand = function(self)
		c = self:GetChildren();
		c.Cursors = {};
		c.Cursors[PLAYER_1] = c["Cursor P1"];
		c.Cursors[PLAYER_2] = c["Cursor P2"];

		for l in ivalues(Letters) do
			local Name = MapLetterToName[l];
			assert(Name);
			Keys[l] = {
				Text = c[Name];
				Width = c[Name]:GetWidth();
				Height = c[Name]:GetHeight();
			};
			assert(c[Name], Name);
		end;

		-- Position letters.
		local RowWidths = {};
		local fX = 0;
		local fXPadding = 4;
		local fY = 0;
		local iRow = 1;
		for l in ivalues(Letters) do
			if l == "O" or l == "0" or l == "BACK" then
				fX = 0;
				fY = fY + 30;
				iRow = iRow + 1;
			end;
			Keys[l].Row = iRow;
			Keys[l].Text:x(fX + Keys[l].Width/2);
			Keys[l].Text:y(fY + Keys[l].Height/2);
			fX = fX + Keys[l].Width;
			RowWidths[iRow] = fX;
			fX = fX + fXPadding;
		end;
		for l in ivalues(Letters) do
			local iRow = Keys[l].Row;
			Keys[l].Text:addx(-RowWidths[iRow]/2);
		end;

		Keys["BACK"].Text:x(-140);
		Keys["ENTER"].Text:x(140);

		for pn in ivalues(PlayerNumber) do
			local sName = SCREENMAN:GetTopScreen():GetSelection(pn);
			local DefaultKey = "A"
			if #sName > 0 then
				DefaultKey = "ENTER"
			end

			self:playcommand("SelectKey", { Key = DefaultKey, PlayerNumber = pn });
			c.Cursors[pn]:finishtweening();
		end
	end;

	CodeMessageCommand = function(self, param)
		local pn = param.PlayerNumber;
		if not SCREENMAN:GetTopScreen():GetAnyStillEntering() and param.Name == "Enter" then
			if SCREENMAN:GetTopScreen():Finish(pn) then
				c.Enter:play();
			end
			return;
		end
		if SCREENMAN:GetTopScreen():GetFinalized( pn ) then
			return;
		end

		if param.Name == "Left" or param.Name == "Right" then
			local iDir = param.Name == "Left" and -1 or 1;

			local idx = LetterIndexes[Selection[pn]];
			idx = idx + iDir;
			idx = math.mod(idx+#Letters-1, #Letters)+1;
			self:playcommand("SelectKey", { Key = Letters[idx], PlayerNumber = pn });
			c.Move:playforplayer(pn);
			return;
		end

		if param.Name == "JumpToEnter" then
			self:playcommand("SelectKey", { Key = "ENTER", PlayerNumber = pn });
			c.Move:playforplayer(pn);
			return;
		end
		if param.Name == "Backspace" then
			if SCREENMAN:GetTopScreen():Backspace(pn) then
				c.Back:playforplayer(pn);
			end
			return;
		end

		if param.Name == "NextRow" or param.Name == "PrevRow" then
			local iDir = param.Name == "PrevRow" and -1 or 1;
			local idx = LetterIndexes[Selection[pn]];
			local iRow = Keys[Selection[pn]].Row;
			iRow = iRow + iDir;
			iRow = math.mod(iRow+4-1, 4)+1;

			local NearestLetter
			local NearestLetterDistance
			for l in ivalues(Letters) do
				if iRow == Keys[l].Row then
					local Text = Keys[l].Text;

					local fDist = distance( PlayerX[pn], PlayerY[pn],
						Text:GetX(), Text:GetY() );
					if not NearestLetterDistance or fDist < NearestLetterDistance then
						NearestLetterDistance = fDist;
						NearestLetter = l;
					end
				end
			end
			assert( NearestLetter );
			self:playcommand("SelectKey", { Key = NearestLetter, PlayerNumber = pn, NoStore = true });

			c.Move:playforplayer(pn);

			return;
		end

		if param.Name == "Enter" then
			if Selection[pn] == "BACK" then
				if SCREENMAN:GetTopScreen():Backspace(pn) then
					Keys[Selection[pn]].Text:playcommand( "Pulse" );
					c.Back:playforplayer(pn);
				end
			elseif Selection[pn] == "ENTER" then
				if SCREENMAN:GetTopScreen():Finish(pn) then
					Keys[Selection[pn]].Text:playcommand( "Pulse" );
					c.Enter:playforplayer(pn);
				end
			else
				local key = Selection[pn]; -- EnterKey may change this
				if SCREENMAN:GetTopScreen():EnterKey(pn, key) then
					Keys[key].Text:playcommand( "Pulse" );
					c.Type:playforplayer(pn);
				end
			end
		end
	end;

	SelectKeyMessageCommand = function(self, param)
		local key = param.Key;
		local pn = param.PlayerNumber;
		if not Keys[key] then return end

		Selection[pn] = key;
		c.Cursors[pn]:stoptweening();
		c.Cursors[pn]:playcommand( "SetSize", { Width=Keys[key].Width+6; tween=cmd(stoptweening;linear,0.10); } );
		c.Cursors[pn]:x( Keys[key].Text:GetX() );
		c.Cursors[pn]:y( Keys[key].Text:GetY() );
		if not param.NoStore then
			PlayerX[pn] = Keys[key].Text:GetX();
			PlayerY[pn] = Keys[key].Text:GetY();
		end
	end;
	MenuTimerExpiredMessageCommand = function(self, param)
		for pn in ivalues(PlayerNumber) do
			SCREENMAN:GetTopScreen():Finish(pn);
		end
		c.Enter:play();
	end;

	OnCommand = function(self, param)
		for key in ivalues(Letters) do
			local fDist = distance( Keys[key].Text:GetX(), Keys[key].Text:GetY(), 0, 100 );
			local f = cmd(
				diffusealpha,0;zoom,0;
				sleep,0.25 + (fDist / 400);
				decelerate,0.5;
				diffusealpha,1;zoom,0.8);
			f(Keys[key].Text);
		end
	end;

	OffCommand = function(self, param)
		for key in ivalues(Letters) do
			local fDist = distance( Keys[key].Text:GetX(), Keys[key].Text:GetY(), 0, 100 );
			local f = cmd(
				sleep,0.0 + (fDist / 400);
				decelerate,0.5;
				diffusealpha,0;zoom,0);
			f(Keys[key].Text);
		end
	end;
};
