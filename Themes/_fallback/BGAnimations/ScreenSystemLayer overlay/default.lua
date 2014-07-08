local function CreditsText( pn )
	local text = LoadFont(Var "LoadingScreen","credits") .. {
		InitCommand=function(self)
			self:name("Credits" .. PlayerNumberToString(pn))
			ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen");
		end;
		UpdateTextCommand=function(self)
			local str = ScreenSystemLayerHelpers.GetCreditsMessage(pn);
			self:settext(str);
		end;
		UpdateVisibleCommand=function(self)
			local screen = SCREENMAN:GetTopScreen();
			local bShow = true;
			if screen then
				local sClass = screen:GetName();
				bShow = THEME:GetMetric( sClass, "ShowCreditDisplay" );
			end

			self:visible( bShow );
		end
	};
	return text;
end;

--[[ local function PlayerPane( PlayerNumber ) 
	local t = Def.ActorFrame {
		InitCommand=function(self)
			self:name("PlayerPane" .. PlayerNumberToString(PlayerNumber));
	-- 		ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen");
		end
	};
	t[#t+1] = Def.ActorFrame {
		Name = "Background";
		Def.Quad {
			InitCommand=cmd(zoomto,160,28;queuecommand,"On");
			OnCommand=cmd(diffuse,PlayerColor(PlayerNumber);fadebottom,1);
		};
	};
	t[#t+1] = LoadFont("Common","Normal") .. {
		Name = "PlayerText";
		InitCommand=cmd(x,-60;maxwidth,80/0.5;zoom,0.5;queuecommand,"On");
		OnCommand=cmd(playcommand,"Set");
		SetCommand=function(self)
			local profile = PROFILEMAN:GetProfile( PlayerNumber) or PROFILEMAN:GetMachineProfile()
			if profile then
				self:settext( profile:GetDisplayName() );
			else
				self:settext( "NoProf" );
			end
		end;
	};
	return t
end --]]
--
local t = Def.ActorFrame {}
	-- Aux
t[#t+1] = LoadActor(THEME:GetPathB("ScreenSystemLayer","aux"));
	-- Credits
t[#t+1] = Def.ActorFrame {
--[[  	PlayerPane( PLAYER_1 ) .. {
		InitCommand=cmd(x,scale(0.125,0,1,SCREEN_LEFT,SCREEN_WIDTH);y,SCREEN_BOTTOM-16)
	}; --]]
 	CreditsText( PLAYER_1 );
	CreditsText( PLAYER_2 ); 
};
	-- Text

-- Minor text formatting functions from Kyzentun.
-- TODO:  Figure out why BitmapText:maxwidth doesn't do what I want.
local function width_limit_text(text, limit, natural_zoom)
	natural_zoom= natural_zoom or 1
	if text:GetWidth() * natural_zoom > limit then
		text:zoomx(limit / text:GetWidth())
	else
		text:zoomx(natural_zoom)
	end
end

local function width_clip_text(text, limit)
	local full_text= text:GetText()
	local fits= text:GetZoomedWidth() <= limit
	local prev_max= #full_text - 1
	local prev_min= 0
	if not fits then
		while prev_max - prev_min > 1 do
			local new_max= math.round((prev_max + prev_min) / 2)
			text:settext(full_text:sub(1, 1+new_max))
			if text:GetZoomedWidth() <= limit then
				prev_min= new_max
			else
				prev_max= new_max
			end
		end
		text:settext(full_text:sub(1, 1+prev_min))
	end
end

local function width_clip_limit_text(text, limit, natural_zoom)
	natural_zoom= natural_zoom or text:GetZoomY()
	local text_width= text:GetWidth() * natural_zoom
	if text_width > limit * 2 then
		text:zoomx(natural_zoom * .5)
		width_clip_text(text, limit)
	else
		width_limit_text(text, limit, natural_zoom)
	end
end

local errorbg
local err_frame
local text_actors= {}
local line_height= 12 -- A good line height for Common Normal at .5 zoom.
local line_width= SCREEN_WIDTH - 20

local next_message_actor= 1

local min_message_time= { show= 4, hide= .125}
local message_time= {
	show= min_message_time.show, hide= min_message_time.hide}

function SetOverlayMessageTime(t, which)
	if not min_message_time[which] then
		SCREENMAN:SystemMessage("Attempted to set invalid overlay message time field: " .. tostring(which))
		return
	end
	if t < min_message_time[which] then
		SCREENMAN:SystemMessage(
			"Attempted to set overlay message " .. which ..
				" time to below minimum of " .. min_message_time[which] .. ".")
		return
	end
	message_time[which]= t
end

local errbg_actor= LoadActor(THEME:GetPathB("ScreenSystemLayer", "errorbg"))
-- Force the name of the returned actor so that we can easily use playcommand on it later.
errbg_actor.Name= "errorbg"

local frame_args= {
	Name="Error frame",
	InitCommand= function(self)
		err_frame= self
		errorbg= self:GetChild("errorbg")
		self:y(-SCREEN_HEIGHT)
	end,
	SystemMessageMessageCommand = function(self, params)
		err_frame:stoptweening()
		err_frame:visible(true)
		local covered_height= line_height * (next_message_actor-1)
		err_frame:y(-SCREEN_HEIGHT + covered_height)
		if errorbg then
			errorbg:playcommand("SetCoveredHeight", {height= covered_height})
		end
		err_frame:sleep(message_time.show)
		err_frame:queuecommand("DecNextActor")
		-- Shift the text on all the actors being shown up by one.
		for i= next_message_actor, 1, -1 do
			if text_actors[i] then
				text_actors[i]:visible(true)
				if i > 1 and text_actors[i-1] then
					text_actors[i]:settext(text_actors[i-1]:GetText())
				else
					text_actors[i]:settext(params.Message)
				end
				width_clip_limit_text(text_actors[i], line_width)
			end
		end
		if next_message_actor <= #text_actors and not params.NoAnimate then
			next_message_actor= next_message_actor + 1
		end
	end,
	DecNextActorCommand= function(self)
		self:linear(message_time.hide)
		self:y(self:GetY()-line_height)
		if text_actors[next_message_actor] then
			text_actors[next_message_actor]:visible(false)
		end
		next_message_actor= next_message_actor - 1
		if next_message_actor > 1 then
			self:queuecommand("DecNextActor")
		else
			self:queuecommand("Off")
		end
	end,
	OffCommand= cmd(visible,false),
	HideSystemMessageMessageCommand = cmd(finishtweening),
	errbg_actor,
}
-- Create enough text actors that we can fill the screen.
local num_text= SCREEN_HEIGHT / line_height
for i= 1, num_text do
	frame_args[#frame_args+1]= 	LoadFont("Common","Normal") .. {
		Name="Text" .. i,
		InitCommand= function(self)
			-- Put them in the list in reverse order so the ones at the bottom of the screen are used first.
			text_actors[num_text-i+1]= self
			self:horizalign(left)
			self:vertalign(top)
			self:x(SCREEN_LEFT + 10)
			self:y(SCREEN_TOP + (line_height * (i-1)) + 2)
			self:shadowlength(1)
			self:zoom(.5)
			self:visible(false)
		end,
		OffCommand= cmd(visible,false),
	}
end
t[#t+1] = Def.ActorFrame(frame_args)

return t
