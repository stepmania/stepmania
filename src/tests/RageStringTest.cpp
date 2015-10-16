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
