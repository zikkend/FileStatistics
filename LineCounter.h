#pragma once

#include <string>
#include <mutex>
#include <filesystem>
#include <future>

#include "LineCountStatistic.h"

class LineCounter
{
public:

	void ProcessFile(const std::filesystem::directory_entry& entry);
	void ProcessDirectory(const std::string& root_folder);

	void ProcessFileConcurrently(const std::filesystem::directory_entry& entry, std::vector<std::future<void>>& tasks);
	void ProcessDirectoryConcurrently(const std::string& root_folder);

	void SaveToStream(std::ostream& os, std::chrono::duration<double>);
	
private:
	LineCountStatistic m_total_line_counts;
	size_t m_total_files = 0;
	std::mutex m_mutex;
	
	void CountLinesInFile(const std::string& file_path);
	bool static IsCppOrCFile(const std::string& extension);
};

