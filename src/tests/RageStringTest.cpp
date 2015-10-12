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
