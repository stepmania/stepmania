#include "RageColorFixture.hpp"

using Rage::Color;

TEST_F(RageColorFixture, addition)
{
	Color superWhite = { 1, 1, 1, 3 };
	
	EXPECT_EQ(superWhite, red + green + blue);
}

TEST_F(RageColorFixture, from_string_comma_valid_red_not_alpha)
{
	Color tmp{};
	bool wasSuccessful = tmp.FromString("1,0,0");
	
	EXPECT_EQ(true, wasSuccessful);
	EXPECT_EQ(red, tmp);
}

TEST_F(RageColorFixture, from_string_comma_invalid_positive_red_not_alpha)
{
	Color tmp{};
	bool wasSuccessful = tmp.FromString("1.1,0,0");
	
	EXPECT_EQ(false, wasSuccessful);
}

TEST_F(RageColorFixture, from_string_comma_invalid_negative_red_not_alpha)
{
	Color tmp{};
	bool wasSuccessful = tmp.FromString("-1,0,0");
	
	EXPECT_EQ(false, wasSuccessful);
}

TEST_F(RageColorFixture, from_string_comma_invalid_positive_green_not_alpha)
{
	Color tmp{};
	bool wasSuccessful = tmp.FromString("0,1.1,0");
	
	EXPECT_EQ(false, wasSuccessful);
}

TEST_F(RageColorFixture, from_string_comma_invalid_negative_green_not_alpha)
{
	Color tmp{};
	bool wasSuccessful = tmp.FromString("0,-1,0");
	
	EXPECT_EQ(false, wasSuccessful);
}

TEST_F(RageColorFixture, from_string_comma_invalid_positive_blue_not_alpha)
{
	Color tmp{};
	bool wasSuccessful = tmp.FromString("0,0,1.1");
	
	EXPECT_EQ(false, wasSuccessful);
}

TEST_F(RageColorFixture, from_string_comma_invalid_negative_blue_not_alpha)
{
	Color tmp{};
	bool wasSuccessful = tmp.FromString("0,0,-1");
	
	EXPECT_EQ(false, wasSuccessful);
}

TEST_F(RageColorFixture, from_string_comma_too_few)
{
	Color tmp{};
	bool wasSuccessful = tmp.FromString("0,1");
	
	EXPECT_EQ(false, wasSuccessful);
}

TEST_F(RageColorFixture, from_string_comma_too_many)
{
	Color tmp{};
	bool wasSuccessful = tmp.FromString("0,0.25,.5,.75,1");
	
	EXPECT_EQ(false, wasSuccessful);
}

TEST_F(RageColorFixture, from_string_hex_invalid_red_hex_alpha)
{
	Color tmp{};
	bool wasSuccessful = tmp.FromString("#FGFFFFFF");
	
	EXPECT_EQ(false, wasSuccessful);
}

TEST_F(RageColorFixture, from_string_hex_invalid_green_hex_alpha)
{
	Color tmp{};
	bool wasSuccessful = tmp.FromString("#FFFGFFFF");
	
	EXPECT_EQ(false, wasSuccessful);
}

TEST_F(RageColorFixture, from_string_hex_invalid_blue_hex_alpha)
{
	Color tmp{};
	bool wasSuccessful = tmp.FromString("#FFFFFGFF");
	
	EXPECT_EQ(false, wasSuccessful);
}

TEST_F(RageColorFixture, from_string_hex_invalid_alpha_hex_alpha)
{
	Color tmp{};
	bool wasSuccessful = tmp.FromString("#FFFFFFFG");
	
	EXPECT_EQ(false, wasSuccessful);
}

TEST_F(RageColorFixture, from_string_hex_invalid_red_not_alpha)
{
	Color tmp{};
	bool wasSuccessful = tmp.FromString("#FGFFFF");
	
	EXPECT_EQ(false, wasSuccessful);
}

TEST_F(RageColorFixture, from_string_hex_invalid_green_not_alpha)
{
	Color tmp{};
	bool wasSuccessful = tmp.FromString("#FFFGFF");
	
	EXPECT_EQ(false, wasSuccessful);
}

TEST_F(RageColorFixture, from_string_hex_invalid_blue_not_alpha)
{
	Color tmp{};
	bool wasSuccessful = tmp.FromString("#FFFFFG");
	
	EXPECT_EQ(false, wasSuccessful);
}

TEST_F(RageColorFixture, from_string_valid_hex_not_alpha)
{
	Color target = { 200 / 255.f, 100 / 255.f, 50 / 255.f, 1 };
	Color tmp{};
	
	bool wasSucessful = tmp.FromString("#C86432");
	
	EXPECT_EQ(true, wasSucessful);
	EXPECT_EQ(target, tmp);
}

TEST_F(RageColorFixture, from_string_valid_hex_alpha)
{
	Color target = { 25 / 255.f, 50 / 255.f, 100 / 255.f, 200 / 255.f };
	Color tmp{};
	
	bool wasSuccessful = tmp.FromString("#193264C8");
	
	EXPECT_EQ(true, wasSuccessful);
	EXPECT_EQ(target, tmp);
}

TEST_F(RageColorFixture, from_string_valid_max_hex)
{
	Color target = { 1, 1, 1, 1 };
	Color tmp{};
	
	bool wasSuccessful = tmp.FromString("#FFFFFFFF");
	
	EXPECT_EQ(true, wasSuccessful);
	EXPECT_EQ(target, tmp);
}

TEST_F(RageColorFixture, to_string_alpha)
{
	Color target = { 25 / 255.f, 50 / 255.f, 100 / 255.f, 200 / 255.f };
	std::string result = target.ToString();
	
	EXPECT_EQ("#193264C8", result);
}

TEST_F(RageColorFixture, to_string_not_alpha)
{
	Color target = { 200 / 255.f, 100 / 255.f, 50 / 255.f, 1 };
	std::string result = target.ToString();
	
	EXPECT_EQ("#C86432", result);
}
