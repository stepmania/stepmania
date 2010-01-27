-- theme identification:
themeInfo = {
	ProductCode = "SSC-00X",
	Name = "Urban Fragments",
	Version = "v0.00",
	Date = "00000000",
	Internal = "dev 00000000-0000 | i0000",
};

-- VersionCheck()
-- Checks the version of StepMania in a fashion that (hopefully)
-- all SM4 versions can understand. Returns true if not SVN.
function VersionCheck()
	local r = SortOrder:Reverse();
	if r['SortOrder_Roulette'] ~= 18 then return true; end;

	-- fallthrough; we should get here if using SVN.
	return false;
end;