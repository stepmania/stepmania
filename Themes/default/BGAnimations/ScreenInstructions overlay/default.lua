local pm = GAMESTATE:GetPlayMode();
local Title = LoadFont( "Common", "normal" ) .. {
	InitCommand = cmd(zoom,.95;addy,-120;diffusetopedge,color("#D3FFFF");diffusebottomedge,color("#0E3C84"));
	Text = THEME:GetString( "ScreenInstructions", pm .. " title" );
};
local InstructionsZoom = .5;
local Instructions = LoadFont( "Common", "normal" ) .. {
	Text = THEME:GetString( "ScreenInstructions", pm .. " instructions" );
	InitCommand = cmd(zoom,InstructionsZoom;addy,80;vertalign,'VertAlign_Bottom';wrapwidthpixels,480/InstructionsZoom;shadowlength,1);
};

local Background = {
	PlayMode_Regular = "_regular";
	PlayMode_Nonstop = "_nonstop";
	PlayMode_Oni = "_oni";
	PlayMode_Endless = "_endless";
	PlayMode_Rave = "_regular";
	PlayMode_Battle = "_regular";
};

local t = Def.ActorFrame {
	LoadActor( Background[pm] );
	Title;
	Instructions; -- Keep this at index 3 or change below.
	InitCommand = cmd(x,SCREEN_LEFT-SCREEN_WIDTH;y,SCREEN_CENTER_Y);
	OnCommand = cmd(sleep,0.4;decelerate,0.6;x,SCREEN_CENTER_X);
	OffCommand = cmd(stoptweening;accelerate,0.3;x,SCREEN_RIGHT+SCREEN_WIDTH);
};


local sGame = GAMESTATE:GetCurrentGame():GetName();
local sButton = '';
if sGame == 'dance' then sButton = 'Down'
elseif sGame == 'pump' then sButton = 'Center'
end

if pm == 'PlayMode_Regular' and sButton ~= '' then
	local noteOffsetY = -30;
	local noteOffsetX = 160;
	local NameFont = LoadFont( "Common", "normal" ) .. {
		InitCommand = cmd(zoom,.8;addy,-90;diffusetopedge,color("#D3FFFF");diffusebottomedge,color("#0E3C84"));
	};
	local InstructionZoom = 0.4;
	local InstructionFont = LoadFont( "Common", "normal" ) .. {
		InitCommand = cmd(horizalign,'HorizAlign_Left';zoom,InstructionZoom;addy,30;wrapwidthpixels,160/InstructionZoom);
	}

	-- Add Hold.
	local holdDelta = NOTESKIN:GetMetricIForNoteSkin( "NoteDisplay", "StartDrawingHoldBodyOffsetFromHead", "default" ) + NOTESKIN:GetMetricIForNoteSkin( "NoteDisplay", "StopDrawingHoldBodyOffsetFromTail", "default" );
	local top = NOTESKIN:LoadActorForNoteSkin( sButton, "Hold Head Inactive", "default" ) .. {
		InitCommand = cmd(addx,-noteOffsetX;addy,noteOffsetY+holdDelta/2);
	}
	local bottom = NOTESKIN:LoadActorForNoteSkin( sButton, "Hold BottomCap Inactive", "default" ) .. {
		InitCommand = cmd(addx,-noteOffsetX;addy,noteOffsetY-holdDelta/2);
	};

	if NOTESKIN:GetMetricBForNoteSkin( "NoteDisplay", "HoldHeadIsAboveWavyParts", "default" ) then
		t[#t+1] = bottom;
		t[#t+1] = top;
	else
		t[#t+1] = top;
		t[#t+1] = bottom;
	end
	t[#t+1] = NameFont .. {
		InitCommand = cmd(addx,-noteOffsetX);
		Text = "Hold Arrows";
	};
	t[#t+1] = InstructionFont .. {
		InitCommand = cmd(addx,-240);
		Text = THEME:GetString( "ScreenInstructions", "Hold instructions" );
	};

	-- Add Mine.
	t[#t+1] = NOTESKIN:LoadActorForNoteSkin( sButton, "Tap Mine", "default" ) .. {
		InitCommand = cmd(addx,noteOffsetX;addy,noteOffsetY);
	};
	t[#t+1] = NameFont .. {
		InitCommand = cmd(addx,noteOffsetX);
		Text = "Mines";
	};
	t[#t+1] = InstructionFont .. {
		InitCommand = cmd(addx,80);
		Text = THEME:GetString( "ScreenInstructions", "Mine instructions" );
	};
elseif pm == 'PlayMode_Oni' then
	-- Merge the tables.
	t[3] = t[3] .. {
		InitCommand = function( self )
			local sText = self:GetText();
			local iStart, iEnd = sText:find( "Great", 1, true );
			if( iStart ~= nil ) then
				iStart = iStart - 1;
				local attr = {
					Length = iEnd - iStart;
					Diffuse = color( "#00F000" );
				};
				iStart = mbstrlen( sText:sub(1, iStart) );
				self:AddAttribute( iStart, attr );
			end
	
			iStart, iEnd = sText:find( "NG", 0 );
			if( iStart ~= nil ) then
				iStart = iStart - 1;
				local attr = {
					Length = iEnd - iStart;
					Diffuse = color( "#F00000" );
				};
				iStart = mbstrlen( sText:sub(1, iStart) );
				self:AddAttribute( iStart, attr );
			end
		end;
	};
end
return t;
