-- How actor creation works:
-- Actors are created from tables.
-- The elements in the table are used to set various attributes of the actor
--   or commands that the actor can execute.
-- Unless stated otherwise, assume that all elements are optional, few
--   attributes or commands are required.
-- Different Actor types have different attributes that can be set.
-- Any element with a name that ends in "Command" must be a function and can
--   later be executed by using the playcommand or queuecommand functions on
--   that actor.
-- Any element with a name that ends in "MessageCommand" must be a function
--   and will be executed whenever that message is broadcast.
-- The functions used for commands and messages are automatically passed a
--   "self" parameter, which is the actor.  Message functions are also passed
--   a params table containing parameters that were broadcast with the
--   message.  playcommand can be used to pass a parameters table to the
--   command, but queuecommand cannot.
-- For ActorFrame and actors that inherit from it, array elements of the
--   table are added as children.
-- Any element in the table that is not recognized as an attribute, command,
--   or child is discarded.

-- Some actors also fetch metrics from metrics.ini to control some aspect.
--   Metrics loaded will be listed in entries for individual actor types.
-- There are some helper functions like LoadActor and LoadFont, which are
--   documented in Docs/Luadoc/lua.xml.

-- This is a simple actor with no visual aspect and basic commands.
Def.Actor{
	-- Attributes:
	-- You should always supply a name for an actor.  The name is used when
	--   finding the actor with GetChild.
	Name= "simple",

	-- The base rotation and zoom.  X, Y, and Z are all settable, though only
	--   X is listed in this example.  Rotation is in degrees.
	BaseRotationX= 15,
	BaseZoomX= .5,

	-- The Init command is run for every actor after all the attributes are set
	--   and all children have been loaded.
	-- It's important to note that the Init command runs during screen
	--   creation, before the screen has changed.  If you use
	--   SCREENMAN:GetTopScreen() in the Init command, you will not get the
	--   screen that is being created because it isn't finished being created.
	InitCommand= function(self)
		Trace("Init command running for " .. self:GetName())
	end,
	-- The On command runs after screen creation has finished.
	OnCommand= function(self)
		Trace("On command running for " .. self:GetName())
		-- Queue the Exa command to execute when all tweening has finished.
		self:queuecommand("Exa")
	end,
	ExaCommand= function(self)
		Trace("Exa command running for " .. self:GetName())
		-- Execute the Tera command right now.
		self:playcommand("Tera", {foo= 1, bar= 2})
	end,
	TeraCommand= function(self, params)
		Trace("Tera command running for " .. self:GetName() .. ": " .. params.foo .. ", " .. params.bar)
		-- Broadcast a message that will execute the Giga command for every actor
		--   that has it.
		MESSAGEMAN:Broadcast("Giga", {foo= 1, bar= 2})
	end,
	GigaMessageCommand= function(self, params)
		Trace("Giga command running for " .. self:GetName() .. ": " .. params.foo .. ", " .. params.bar)
	end
}

-- It is important to note that the things in Def are functions and you are
-- passing them a table when you use them.  "Def.Actor{}", "Def.Actor({})",
-- and "local foo= {}  Def.Actor(foo)" are all equivalent in effect, but
-- some forms make is easier to write well organized code.
