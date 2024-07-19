#include "LineCountStatistic.h"

std::ostream& operator<<(std::ostream& os, const LineCountStatistic& line_count)
{
    return os << "Line Count Statistics\nBlank lines: " << line_count.blank_lines
        << "\nCode lines: " << line_count.code_lines
        << "\nComment lines: " << line_count.comment_lines;
}