--[[
-- 32 lines of goodness.
-- Links to original source of code and usage documentation:
-- http://love2d.org/wiki/32_lines_of_goodness
-- http://love2d.org/forums/viewtopic.php?f=5&t=3344
-- Patches by Robin and FSX (which bring it to 41 lines)
--]]

local mt_class = {__index = mt_class, __call = define}

function mt_class:extends(parent)
   self.super = parent
   setmetatable(self, {__index = parent, __call=define})
   parent.__members__ = parent.__members__ or {}
   return self
end

local function define(class, members)
   class.__members__ = class.__members__ or {}
   local function copyMembersDown(origin,cls)
      if cls.super then
	     for k,v in pairs(cls.super.__members__) do
	        origin[k] = v
         end
	     return copyMembersDown(origin, cls.super)
	  end
   end
   copyMembersDown(class.__members__,class)
   for k, v in pairs(members) do
      class.__members__[k] = v
   end
   function class:new(...)
      local newvalue = {}
      for k, v in pairs(class.__members__) do
         newvalue[k] = v
      end
      setmetatable(newvalue, {__index = class})
      if newvalue.__init then
         newvalue:__init(...)
      end
      return newvalue
   end
end

function class(name)
    local newclass = {}
   _G[name] = newclass
   return setmetatable(newclass, mt_class)
end