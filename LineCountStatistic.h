#pragma once

#include <iostream>

struct LineCountStatistic
{
	size_t blank_lines = 0;
	size_t code_lines = 0;
	size_t comment_lines = 0;

	constexpr LineCountStatistic& operator+=(const LineCountStatistic& rhs)
	{
		blank_lines += rhs.blank_lines;
		code_lines += rhs.code_lines;
		comment_lines += rhs.comment_lines;

		return *this;
	}
};

std::ostream& operator<<(std::ostream& os, const LineCountStatistic& line_count);
