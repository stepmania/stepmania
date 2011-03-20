local t = Def.ActorFrame {};
--[[ t[#t+1] = Def.Actor { 
	OnCommand=function(self)
		local sTarget = PROFILEMAN:GetProfileDir('ProfileSlot_Machine');
		--
		WriteFile(sTarget .. "Jason.txt","Charlie is a magical unicorn!");
	end;
}; --]]

return t