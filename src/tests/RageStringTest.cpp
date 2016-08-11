#include "RageString.hpp"
#include "gtest/gtest.h"

GTEST_TEST(RageString, head_length_beyond_string)
{
	std::string stepmania{"Stepmania"};

	std::string target = Rage::head(stepmania, 10);

	EXPECT_EQ(stepmania, target);
}

GTEST_TEST(RageString, head_root_dir_check)
{
	std::string commonHomePath{"/home/stepmans"};

	std::string target = Rage::head(commonHomePath, 1);

	EXPECT_EQ("/", target);
}

GTEST_TEST(RageString, head_best_courses_check)
{
	std::string commonBest{"BEST 4"};

	std::string target = Rage::head(commonBest, 4);

	EXPECT_EQ("BEST", target);
}

GTEST_TEST(RageString, head_command_at_end_check)
{
	std::string usefulCommand{"UpgradeToStepMania5Command"};

	std::string target = Rage::head(usefulCommand, -7);

	EXPECT_EQ("UpgradeToStepMania5", target);
}

GTEST_TEST(RageString, tail_length_beyond_string)
{
	std::string stepmania{"Stepmania"};

	std::string target = Rage::tail(stepmania, 10);

	EXPECT_EQ(stepmania, target);
}

GTEST_TEST(RageString, tail_command_verification_check)
{
	std::string usefulCommand{"UpgradeToStepMania5Command"};

	std::string target = Rage::tail(usefulCommand, 7);

	EXPECT_EQ("Command", target);
}

GTEST_TEST(RageString, tail_song_group_name_check)
{
	std::string usefulCommand{"/Songs/StepMix 5"};
	
	std::string target = Rage::tail(usefulCommand, -7);
	
	EXPECT_EQ("StepMix 5", target);
}

GTEST_TEST(RageString, hexify_ascii)
{
	wchar_t test = 'a';

	std::string target = Rage::hexify(test, 4);

	EXPECT_EQ("0061", target);
}

GTEST_TEST(RageString, ci_already_lower_two_vars)
{
	Rage::ci_ascii_string target{"hello"};
	Rage::ci_ascii_string other{"hello"};
	EXPECT_EQ(target == other, true);
}

GTEST_TEST(RageString, ci_already_lower_one_var_c)
{
	Rage::ci_ascii_string target{"hello"};
	EXPECT_EQ(target == "hello", true);
}

GTEST_TEST(RageString, ci_already_lower_one_var_std)
{
	Rage::ci_ascii_string target{"hello"};
	std::string other{"hello"};
	EXPECT_EQ(target == other, true);
}

GTEST_TEST(RageString, ci_upper_vs_lower_two_vars)
{
	Rage::ci_ascii_string target{"HOWDY"};
	Rage::ci_ascii_string other{"howdy"};
	EXPECT_EQ(target == other, true);
}

GTEST_TEST(RageString, ci_upper_vs_lower_one_var_c)
{
	Rage::ci_ascii_string target{"HOWDY"};
	EXPECT_EQ(target == "howdy", true);
}

GTEST_TEST(RageString, ci_upper_vs_lower_one_var_std)
{
	Rage::ci_ascii_string target{"HOWDY"};
	std::string other{"howdy"};
	EXPECT_EQ(target == other, true);
}

GTEST_TEST(RageString, ci_mixed_all_over_two_vars)
{
	Rage::ci_ascii_string target{"HoLa"};
	Rage::ci_ascii_string other{"hOlA"};
	EXPECT_EQ(target == other, true);
}

GTEST_TEST(RageString, ci_mixed_all_over_one_var_c)
{
	Rage::ci_ascii_string target{"HoLa"};
	EXPECT_EQ(target == "hOlA", true);
}

GTEST_TEST(RageString, ci_mixed_all_over_one_var_std)
{
	Rage::ci_ascii_string target{"HoLa"};
	std::string other{"hOlA"};
	EXPECT_EQ(target == other, true);
}

GTEST_TEST(RageString, ci_extensions_same_eq)
{
	Rage::ci_ascii_string ini{".ini"};
	Rage::ci_ascii_string other{".ini"};
	
	EXPECT_EQ(ini == other, true);
}

GTEST_TEST(RageString, ci_extensions_same_ne)
{
	Rage::ci_ascii_string ini{".ini"};
	Rage::ci_ascii_string other{".ini"};
	
	EXPECT_EQ(ini != other, false);
}

GTEST_TEST(RageString, ci_extensions_different_eq)
{
	Rage::ci_ascii_string ini{".ini"};
	Rage::ci_ascii_string png{".png"};
	
	EXPECT_EQ(ini == png, false);
}

GTEST_TEST(RageString, ci_extensions_different_ne)
{
	Rage::ci_ascii_string ini{".ini"};
	Rage::ci_ascii_string png{".png"};
	
	EXPECT_EQ(ini != png, true);
}

GTEST_TEST(RageString, starts_with_sanity)
{
	std::string stepmania{"Stepmania"};
	EXPECT_EQ(Rage::starts_with(stepmania, "Step"), true);
}

GTEST_TEST(RageString, ends_with_sanity)
{
	std::string stepmania{"Stepmania"};
	EXPECT_EQ(Rage::ends_with(stepmania, "mania"), true);
}

GTEST_TEST(RageString, replace_char)
{
	std::string tongueTwister{ "She sells seashells by the seashore." };
	std::string answer{ "Sxe sells seasxells by txe seasxore." };
	Rage::replace(tongueTwister, 'h', 'x');
	EXPECT_EQ(tongueTwister == answer, true);
}

GTEST_TEST(RageString, replace_std_string)
{
	std::string tongueTwister{ "She sells seashells by the seashore." };
	std::string answer{ "She sells sayshells by the sayshore." };
	Rage::replace(tongueTwister, "sea", "say");
	EXPECT_EQ(tongueTwister == answer, true);
}

void test_casing(std::string const &base, std::string target, std::string (*rageFunc)(std::string const &));
void test_casing(std::string const &base, std::string target, std::string (*rageFunc)(std::string const &))
{
	EXPECT_EQ(base == rageFunc(target), true);
}

GTEST_TEST(RageString, make_upper_already_upper)
{
	::test_casing("STEPMANIA", "STEPMANIA", &Rage::make_upper);
}

GTEST_TEST(RageString, make_upper_mixed_ascii)
{
	::test_casing("STEPMANIA", "StepMania", &Rage::make_upper);
}

GTEST_TEST(RageString, make_upper_all_lower)
{
	::test_casing("STEPMANIA", "stepmania", &Rage::make_upper);
}

GTEST_TEST(RageString, make_upper_senorita)
{
	// Convert Señorita to SEÑORITA.
	::test_casing("SE\xc3\x91ORITA", "Se\xc3\xb1orita", &Rage::make_upper);
}

GTEST_TEST(RageString, make_lower_already_lower)
{
	::test_casing("stepmania", "stepmania", &Rage::make_lower);
}

GTEST_TEST(RageString, make_lower_mixed_ascii)
{
	::test_casing("stepmania", "sTEPmANIA", &Rage::make_lower);
}

GTEST_TEST(RageString, make_lower_all_upper)
{
	::test_casing("stepmania", "STEPMANIA", &Rage::make_lower);
}

GTEST_TEST(RageString, make_lower_vertex_squared)
{
	// Convert VERTEX² to vertex².
	::test_casing("vertex\xc2\xb2", "VERTEX\xc2\xb2", &Rage::make_lower);
}

GTEST_TEST(RageString, join_nothing)
{
	std::vector<std::string> nothing;
	EXPECT_EQ(Rage::join(",", nothing) == "", true);
}

GTEST_TEST(RageString, join_single)
{
	std::vector<std::string> single { "hello" };
	EXPECT_EQ(Rage::join("-", single) == "hello", true);
}

GTEST_TEST(RageString, join_multiple)
{
	std::vector<std::string> filetypes { "ssc", "sm", "dwi", "bms", "ksf" };
	std::string target = Rage::join("^^", filetypes);
	EXPECT_EQ(target == "ssc^^sm^^dwi^^bms^^ksf", true);
}

GTEST_TEST(RageString, join_multiple_first_three)
{
	std::vector<std::string> filetypes { "ssc", "sm", "dwi", "bms", "ksf" };
	std::string target = Rage::join("^^", filetypes.begin(), filetypes.begin() + 3);
	EXPECT_EQ(target == "ssc^^sm^^dwi", true);
}

GTEST_TEST(RageString, join_multiple_middle_three)
{
	std::vector<std::string> filetypes { "ssc", "sm", "dwi", "bms", "ksf" };
	std::string target = Rage::join("^^", filetypes.begin() + 1, filetypes.begin() + 4);
	EXPECT_EQ(target == "sm^^dwi^^bms", true);
}

GTEST_TEST(RageString, join_multiple_last_three_begin)
{
	std::vector<std::string> filetypes { "ssc", "sm", "dwi", "bms", "ksf" };
	std::string target = Rage::join("^^", filetypes.begin() + 2, filetypes.begin() + 5);
	EXPECT_EQ(target == "dwi^^bms^^ksf", true);
}

GTEST_TEST(RageString, join_multiple_last_three_end)
{
	std::vector<std::string> filetypes { "ssc", "sm", "dwi", "bms", "ksf" };
	std::string target = Rage::join("^^", filetypes.begin() + 2, filetypes.end());
	EXPECT_EQ(target == "dwi^^bms^^ksf", true);
}

GTEST_TEST(RageString, split_empty)
{
	std::vector<std::string> nothing;
	std::string empty{""};
	EXPECT_EQ(nothing.size() == Rage::split(empty, " ").size(), true);
}

// TODO: Find a way to utilize Google Mock for the remaining split tests.
GTEST_TEST(RageString, split_american_phone)
{
	std::string phone{"1-800-555-3253"}; // AKA, should be FAKE
	std::vector<std::string> portions{"1", "800", "555", "3253"};
	auto target = Rage::split(phone, "-");
	EXPECT_EQ(portions.size() == target.size(), true);
	for (int i = 0; i < portions.size(); ++i)
	{
		EXPECT_EQ(portions[i] == target[i], true);
	}
}

GTEST_TEST(RageString, split_filetypes_include)
{
	std::string types{"ssc;sm;;dwi"};
	std::vector<std::string> portions{"ssc", "sm", "", "dwi"};
	auto target = Rage::split(types, ";", Rage::EmptyEntries::include);
	EXPECT_EQ(portions.size() == target.size(), true);
	for (int i = 0; i < portions.size(); ++i)
	{
		EXPECT_EQ(portions[i] == target[i], true);
	}
}

GTEST_TEST(RageString, split_filetypes_skip)
{
	std::string types{"ssc;sm;;dwi"};
	std::vector<std::string> portions{"ssc", "sm", "dwi"};
	auto target = Rage::split(types, ";", Rage::EmptyEntries::skip);
	EXPECT_EQ(portions.size() == target.size(), true);
	for (int i = 0; i < portions.size(); ++i)
	{
		EXPECT_EQ(portions[i] == target[i], true);
	}
}

GTEST_TEST(RageString, split_in_place_provided)
{
	std::string str { "a,b,c" };
	int start = 0;
	int size = -1;
	for(;;)
	{
		Rage::split_in_place(str, ",", start, size);
		if (start == str.size())
		{
			break;
		}
		str[start] = 'Q';
	}
	EXPECT_EQ(str == "Q,Q,Q", true);
}

GTEST_TEST(RageString, split_in_place_simple)
{
	std::string types{"ssc;sm;;dwi"};
	int start = 0;
	int size = -1;
	Rage::split_in_place(types, ";", start, size);
	EXPECT_EQ(size == 3, true);
}

GTEST_TEST(RageString, split_in_place_advanced_skip)
{
	std::string types{"ssc;sm;;dwi"};
	int start = 5;
	int size = -1;
	Rage::split_in_place(types, ";", start, size);
	EXPECT_EQ(size == 1, true);
}

GTEST_TEST(RageString, split_in_place_advanced_skip_2)
{
	std::string types{"ssc;sm;;dwi"};
	int start = 6;
	int size = -1;
	Rage::split_in_place(types, ";", start, size);
	EXPECT_EQ(size == 3, true);
}

GTEST_TEST(RageString, split_in_place_advanced_include)
{
	std::string types{"ssc;sm;;dwi"};
	int start = 5;
	int size = -1;
	Rage::split_in_place(types, ";", start, size, Rage::EmptyEntries::include);
	EXPECT_EQ(size == 1, true);
}

GTEST_TEST(RageString, split_in_place_advanced_include_2)
{
	std::string types{"ssc;sm;;dwi"};
	int start = 6;
	int size = -1;
	Rage::split_in_place(types, ";", start, size, Rage::EmptyEntries::include);
	EXPECT_EQ(size == 0, true);
}

GTEST_TEST(RageString, trim_left_whitespace)
{
	std::string initial{" \r\n\t\r\n  hello"};
	std::string target = Rage::trim_left(initial);
	EXPECT_EQ(target == "hello", true);
}

GTEST_TEST(RageString, trim_left_custom_chars)
{
	std::string initial{"qxqxqxStepMania"};
	std::string target = Rage::trim_left(initial, "qx");
	EXPECT_EQ(target == "StepMania", true);
}

GTEST_TEST(RageString, trim_right_whitespace)
{
	std::string initial{"hello \t\n\r\t\n\r   "};
	std::string target = Rage::trim_right(initial);
	EXPECT_EQ(target == "hello", true);
}

GTEST_TEST(RageString, trim_right_custom_chars)
{
	std::string initial{"StepManiaqxqxqxq"};
	std::string target = Rage::trim_right(initial, "qx");
	EXPECT_EQ(target == "StepMania", true);
}

GTEST_TEST(RageString, trim_whitespace)
{
	std::string initial{" \t\nhello\r\n  \t"};
	std::string target = Rage::trim(initial);
	EXPECT_EQ(target == "hello", true);
}

GTEST_TEST(RageString, trim_custom_chars)
{
	std::string initial{"xqxStepManiaqxq"};
	std::string target = Rage::trim(initial, "xq");
	EXPECT_EQ(target == "StepMania", true);
}

GTEST_TEST(RageString, base_name_ends_in_file)
{
	std::string path{"Songs/StepMania 5/Mecha-Tribe Assault/Mecha-Tribe Assault.ssc"};
	std::string base = Rage::base_name(path);
	EXPECT_EQ(base == "Mecha-Tribe Assault.ssc", true);
}

GTEST_TEST(RageString, base_name_ends_in_dir)
{
	std::string path{"Songs/StepMania 5/Mecha-Tribe Assault"};
	std::string base = Rage::base_name(path);
	EXPECT_EQ(base == "Mecha-Tribe Assault", true);
}

GTEST_TEST(RageString, dir_name_ends_in_file)
{
	std::string path{"Songs/StepMania 5/Mecha-Tribe Assault/Mecha-Tribe Assault.ssc"};
	std::string base = Rage::dir_name(path);
	EXPECT_EQ(base == "Songs/StepMania 5/Mecha-Tribe Assault/", true);
}

GTEST_TEST(RageString, dir_name_ends_in_dir)
{
	std::string path{"Songs/StepMania 5/Mecha-Tribe Assault/"};
	std::string base = Rage::dir_name(path);
	EXPECT_EQ(base == "Songs/StepMania 5/", true);
}

GTEST_TEST(RageString, dir_name_single_dir_prefix)
{
	std::string path{"e/"};
	std::string base = Rage::dir_name(path);
	EXPECT_EQ(base == "./", true);
}

GTEST_TEST(RageString, dir_name_single_dir_suffix)
{
	std::string path{"/home"};
	std::string base = Rage::dir_name(path);
	EXPECT_EQ(base == "/", true);
}

GTEST_TEST(RageString, dir_name_root)
{
	std::string path{"/"};
	std::string base = Rage::dir_name(path);
	EXPECT_EQ(base == "/", true);
}
