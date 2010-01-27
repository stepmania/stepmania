local a = math.random(50);
local ret;
if a % 2 == 0 then
	ret = "_agB";
else
	ret = "_agA";
end;
return THEME:GetPathS("",ret);