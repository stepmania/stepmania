--[[
USW Noteskin.lua Version V2
Made for Version Stepmania 5 Preview 4

I am the bone of my noteskin
Arrows are my body, and explosions are my blood
I have created over a thousand noteskins
Unknown to death
Nor known to life
Have withstood pain to create many noteskins
Yet these hands will never hold anything
So as I pray, Unlimited Stepman Works

If you want to know the list of lua commands you can use here is the main page for help http://kki.ajworld.net/lua/sm5/Lua.xml
--]]

local ret = ... or {};

--I always had the problem were I wrote var instead of Var, which really iritated me wondering why a code wouldnt work
--You can edit it out if you want, But I'm just gonna leave it here so incase a person wants to make a noteskin
--And gets the same problem as me were you use var instead of Var, atleast this should make it still work
--But its best to use Var, And make sure to check all your code for little mistakes while you write ;)
local var = Var;

--[[
This is the general redirect table, Not the general redirect code, We use this if we are lazy ;)
Or if we cant a directon of a noteskin use the exact same files as another direction
The most re used direction in this case is "Down"
You can add and remove values if you want, It works for every game type,
If you want an easy way to know how all directions are called go in to stepmaniaoptions and change gametype
Then if you go to key configure look at the names for the keys, They are the same names that are used in the noteskin.lua
--]]
ret.RedirTable =
{
	Up = "Down",
	Down = "Down",
	Left = "Down",
	Right = "Down",
	Center = "Down",
	DownLeft = "Down",
	DownRight = "Down",
	UpLeft = "Down",
	UpRight = "Down",
};

--[[
This is the general redirect code, We use this if we want to redirect parts of noteskins to other parts
Because unlike the ret.RedirTable which redirects everything to the defined direction.
Here we can add redirects for seperate elements which means we can use separate images if we want
--]]
local OldRedir = ret.Redir;
ret.Redir = function(sButton, sElement)
	sButton, sElement = OldRedir(sButton, sElement);
	
	--This is were we call the ret.RedirTable and define it as sButton
	--So it get called when we return sButton including the code under here
	sButton = ret.RedirTable[sButton];
	
		--We want to use custom hold/roll per direction, But keep global hold/roll heads and explosions.
	if string.find(sElement, "Hold") or string.find(sElement, "Roll") then
		if not string.find(sElement, "Head") and not string.find(sElement, "Explosion") then
		if Var "Button" == "Left" then sButton = "Left"; end
		if Var "Button" == "Right" then sButton = "Right"; end
		if Var "Button" == "Down" then sButton = "Down"; end
		if Var "Button" == "Up" then sButton = "Up"; end
		if GAMESTATE:GetCurrentGame():GetName() == "dance" then 
			if Var "Button" == "UpLeft" then sButton = "UpLeft"; end
			if Var "Button" == "UpRight" then sButton = "UpRight"; end
		end
		
		end
	end
	
	--Making Roll Head/Explosion fallback on Hold
	if  sElement == "Roll Head Inactive" then sElement = "Hold Head Inactive"; end 
	if  sElement == "Roll Head Active" then sElement = "Hold Head Active"; end
	if  sElement == "Roll Explosion" then sElement = "Hold Explosion"; end
	
	--Adding stuff for more directions for diffrent gametypes	
	if GAMESTATE:GetCurrentGame():GetName() == "dance" then 
		--Making the Upleft and Upright Hold/Roll body/BottomCap/TopCap but not Explosions use SoloUpLeft and SoloUpRight instead
		--When the gametype is dance this way we can define diffrent images for diffrent gametypes
		if string.find(sElement, "Hold") or string.find(sElement, "Roll") then
			if Var "Button" == "UpLeft" and not string.find(sElement, "Explosion") then sButton = "SoloUpLeft"; end
			if Var "Button" == "UpRight" and not string.find(sElement, "Explosion") then sButton = "SoloUpRight"; end
		end
	elseif GAMESTATE:GetCurrentGame():GetName() == "techno" then
		-- See below for why two options.
		if GAMESTATE:GetMasterPlayerNumber() == "PlayerNumber_P1" then
			if Var "Button" == "Center" then sButton = "Centerp1"; end
		elseif GAMESTATE:GetMasterPlayerNumber() == "PlayerNumber_P2" then
			if Var "Button" == "Center" then sButton = "Centerp2"; end
		end
		if string.find(sButton, "Center") then
			if string.find(sElement, "Bottomcap") then sButton = "Center"; end
			if string.find(sElement, "Topcap") then sButton = "Center"; end
			if string.find(sElement, "Body") then sButton = "Center"; end
		end
		if string.find(sElement, "Explosion") then sButton = "Center"; end
		if sElement == "Tap Explosion Dim" then sElement = "Tap Explosion Bright"; end
		--]]
	elseif GAMESTATE:GetCurrentGame():GetName() == "pump" then 
		--Making Pump it up be rythm color based, Only problem is that in sm player 1 and player 2 noteskins are both defined as player 1
		--Need a way to get the parent but atm it ends up as nil because of no parent
		--So Player 1 and 2 both use Player 1 images for now
		--We also let every direction aside from Center use DownLeftp1
		if GAMESTATE:GetMasterPlayerNumber() == "PlayerNumber_P1" then
			if Var "Button" == "UpLeft" then sButton = "DownLeftp1"; end
			if Var "Button" == "UpRight" then sButton = "DownLeftp1"; end
			if Var "Button" == "DownLeft" then sButton = "DownLeftp1"; end
			if Var "Button" == "DownRight" then sButton = "DownLeftp1"; end
			if Var "Button" == "Center" then sButton = "Centerp1"; end
		elseif GAMESTATE:GetMasterPlayerNumber() == "PlayerNumber_P2" then
			if Var "Button" == "UpLeft" then sButton = "Downleftp2"; end
			if Var "Button" == "UpRight" then sButton = "Downleftp2"; end 
			if Var "Button" == "DownLeft" then sButton = "DownLeftp2"; end
			if Var "Button" == "DownRight" then sButton = "DownLeftp2"; end
			if Var "Button" == "Center" then sButton = "Centerp2"; end
		end
		--We already defined everything for diffrent players above
		--But we want to use the same Center hold images for every direction and player
		if string.find(sElement, "Bottomcap") then sButton = "Center"; end
		if string.find(sElement, "Topcap") then sButton = "Center"; end
		if string.find(sElement, "Body") then sButton = "Center"; end
		--Lets also add explosion as Center
		if string.find(sElement, "Explosion") then sButton = "Center"; end
		if sElement == "Tap Explosion Dim" then sElement = "Tap Explosion Bright"; end
	end
	
	--Define that every direction uses Tap Mine from Down
	if Var "Element" == "Tap Mine" then sButton = "Down"; end
	
	return sButton, sElement;
end

--[[
This is the general function code
In here we can define how we want the stuff to act
Which basicly means instead of using a load of lua files for just some effect
We can just use code in here so we need to use less files
Which is nice if you want to save up space ;)
Only problem is that the Hold/Roll parts are written down in the code as sprite files
Which is the original old 3.9 code, Which basicly means that their code doesnt work in here

Also unlike the general redirect code which has sElement and sButton defined in the common noteskin
They need to be defined here manualy which can be done with 
local sElement = Var "Element";
local sButton = Var "Button";
To make it easier for everyone I already added them
The reason we use local to define the stuff is for when a code doesnt accept the full code
Like for example if we did string.find(Var "Element", "Down") it wouldnt work
--]]
local OldFunc = ret.Load;
function ret.Load()
	local t = OldFunc();
	local sElement = Var "Element";
	local sButton = Var "Button";

	--Explosion should not be rotated; it calls other actors.
	if Var "Element" == "Explosion"	then
		t.BaseRotationZ = nil;
	end
	
	
	if GAMESTATE:GetCurrentGame():GetName() == "dance" then 
	--We define that we dont want the hold heads for UpLeft and UpRight to be rotated because we rotate them in a lua file
		if Var "Button" == "UpLeft" and string.find(sElement, "Head") then t.BaseRotationZ = nil; end
		if Var "Button" == "UpRight" and string.find(sElement, "Head") then t.BaseRotationZ = nil; end
	elseif GAMESTATE:GetCurrentGame():GetName() == "techno" then
		if Var "Element" == "Tap Mine" then t.InitCommand=cmd(zoom,-0.8); end
	elseif GAMESTATE:GetCurrentGame():GetName() == "pump" then 
	--Because the images for Pump it up are using the DownLeft for every direction aside from center, We let it be rotated here
	--Because the rotate table is set up for dance and these are for PIU
		if Var "Button" == "UpLeft" then t.BaseRotationZ = 90; end
		if Var "Button" == "UpRight" then t.BaseRotationZ = 180; end
		if Var "Button" == "DownLeft" then t.BaseRotationZ = nil; end
		if Var "Button" == "DownRight" then t.BaseRotationZ = -90; end	
		if Var "Element" == "Tap Mine" then t.InitCommand=cmd(zoom,-0.8); end
	end
	return t;
end
-- >


--Define which parts of noteskins which we want to rotate
ret.PartsToRotate =
{
	["Receptor"] = true,
	["Tap Explosion Bright"] = true,
	["Tap Explosion Dim"] = true,
	["Tap Note"] = true,
	["Tap Fake"] = true,
	["Tap Lift"] = true,
	["Tap Addition"] = true,
	["Hold Explosion"] = true,
	["Hold Head Active"] = true,
	["Hold Head Inactive"] = true,
	["Roll Explosion"] = true,
	["Roll Head Active"] = true,
	["Roll Head Inactive"] = true,
};
--Defined the parts to be rotated at which degree
ret.Rotate =
{
	Up = 180,
	Down = 0,
	Left = 90,
	Right = -90,
	UpLeft = 135,
	UpRight = -135,
	Center = 0,
	DownLeft = 45,
	DownRight = -45,
};

--Parts that should be Redirected to _Blank.png
--you can add/remove stuff if you want
ret.Blank =
{
	["Hold Tail Active"] = true,
	["Hold Tail Inactive"] = true,
	["Roll Tail Active"] = true,
	["Roll Tail Inactive"] = true,
};

--dont forget to close the ret cuz else it wont work ;>
return ret;
