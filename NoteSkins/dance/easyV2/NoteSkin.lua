--I am the bone of my noteskin
--Arrows are my body, and explosions are my blood
--I have created over a thousand noteskins
--Unknown to death
--Nor known to life
--Have withstood pain to create many noteskins
--Yet these hands will never hold anything
--So as I pray, Unlimited Stepman Works

local ret = ... or {};

--Defining on which direction the other directions should be bassed on
--This will let us use less files which is quite handy to keep the noteskin directory nice
--Do remember this will Redirect all the files of that Direction to the Direction its pointed to
--If you only want some files to be redirected take a look at the "custom hold/roll per direction"
ret.RedirTable =
{	
    --PIU
	Center = "Center",
	DownLeft = "DownLeft", -- for PIU and dance
	DownRight = "DownLeft", -- for PIU and dance
	UpLeft = "DownLeft",
	UpRight = "DownLeft",
	--Dance
	Down = "Down",
	Up = "Down",
	Left = "Down",
	Right = "Down",
};

-- < 
--Between here we usally put all the commands the noteskin.lua needs to do, some are extern in other files
--If you need help with lua go to  http://kki.ajworld.net/lua/ssc/Lua.xml there are a bunch of codes there
--Also check out commen it has a load of lua codes in files there
--Just play a bit with lua its not that hard if you understand coding
--But SM can be an ass in some cases, and some codes jut wont work if you dont have the noteskin on FallbackNoteSkin=common in the metric.ini 
local OldRedir = ret.Redir;
ret.Redir = function(sButton, sElement)
	sButton, sElement = OldRedir(sButton, sElement);
	
	sButton = ret.RedirTable[sButton];
	
	if not string.find(sElement, "Head") and
	not string.find(sElement, "Explosion") then
		if string.find(sElement, "Hold") or
		string.find(sElement, "Roll") then
			if Var "Button" == "Down" or Var "Button" == "Up" or Var "Button" == "Left" or Var "Button" == "Right" then
				sButton = "Down";
			else
				sButton = "Center";
			end
		end
	end

	-- Instead of separate hold heads, use the tap note graphics.
	if sElement == "Hold Head Inactive" or
	   sElement == "Hold Head Active" or
	   sElement == "Roll Head Inactive" or
	   sElement == "Roll Head Active" or
	   sElement == "Tap Fake"
	then
		sElement = "Tap Note";
	end
	
	if sElement == "Tap Explosion Dim" or
	   sElement == "Hold Explosion" or
	   sElement == "Roll Explosion" 
	then
		sElement = "Tap Explosion Bright";
	end
	
	if sElement == "Tap Mine"
	then
		sButton = "Down";
	end

	return sButton, sElement;
end

local OldFunc = ret.Load;
function ret.Load()
	local t = OldFunc();

	--Explosion should not be rotated; it calls other actors.
	if Var "Element" == "Explosion"	then
		t.BaseRotationZ = nil;
	end
	return t;
end
-- >


-- Parts of noteskins which we want to rotate
ret.PartsToRotate =
{
	["Receptor"] = true,
	["Tap Explosion Bright"] = true,
	["Tap Explosion Dim"] = true,
	["Tap Note"] = true,
	["Tap Fake"] = true,
	["Tap Addition"] = true,
	["Hold Explosion"] = true,
	["Hold Head Active"] = true,
	["Hold Head Inactive"] = true,
	["Roll Explosion"] = true,
	["Roll Head Active"] = true,
	["Roll Head Inactive"] = true,
};
-- Defined the parts to be rotated at which degree
ret.Rotate =
{
	--PIU
	Center = 0,
	DownLeft = 0,
	DownRight = -90,
	UpLeft = 90,
	UpRight = 180,
	--Dance
	Down = 0,
	Up = 180,
	Left = 90,
	Right = -90,
};

-- Parts that should be Redirected to _Blank.png
-- you can add/remove stuff if you want
ret.Blank =
{
	["HitMine Explosion"] = true,
	["Hold Tail Active"] = true,
	["Hold Tail Inactive"] = true,
	["Roll Tail Active"] = true,
	["Roll Tail Inactive"] = true,
};

-- dont forget to close the ret cuz else it wont work ;>
return ret;
