#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <chrono>

struct LineCounts
{
	size_t blank_lines = 0;
	size_t code_lines = 0;
	size_t comment_lines = 0;

	constexpr LineCounts& operator+=(const LineCounts& rhs);
};

std::ostream& operator<<(std::ostream& os, const LineCounts& line_count);

void CountLinesInFile(const std::string& file_path, LineCounts& overall_count);

size_t ProcessDirectory(const std::string& root_folder, LineCounts& total_counts);
size_t ProcessDirectoryConcurrently(const std::string& root_folder, LineCounts& total_counts);

void SaveToStream(std::ostream& os, const LineCounts& lc, const size_t total_files, std::chrono::duration<double>);



