ThemeInfo = {
	Name="_Themekit_PIU";
	Version="pretty much final";
	Code="DSMT-PIUKIT";
	Date = {31,7,2010};
};

WORKING_VERSION = (ProductVersion() == "v1.0 rc2")

if GetUserPrefB("OptionsMode") == nil then
	SetUserPref("OptionsMode", false)
end

--no anda así, mejor uso pantallas
--assert(SSC and WORKING_VERSION, "El Themekit PIU es compatible sólo con SM-SSC")

--not working like this, better use screens
--assert(SSC and WORKING_VERSION, "Themekit PIU is compatible only with SM-SSC RC1.5")