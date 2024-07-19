#include "FileUtils.h"

#include <string_view>
#include <algorithm>
#include <filesystem>
#include <mutex>
#include <future>

namespace fs = std::filesystem;

std::mutex g_mutex;

constexpr LineCounts& LineCounts::operator+=(const LineCounts& rhs)
{
    blank_lines += rhs.blank_lines;
    code_lines += rhs.code_lines;
    comment_lines += rhs.comment_lines;

    return *this;
}

std::ostream& operator<<(std::ostream& os, const LineCounts& line_count)
{
    return os << "Line Count Statistics\nBlank lines: " << line_count.blank_lines
        << "\nCode lines: " << line_count.code_lines
        << "\nComment lines: " << line_count.comment_lines;
}

static inline std::wstring_view TrimLeft(const std::wstring& str) {
    auto begin = std::find_if_not(str.begin(), str.end(), ::isspace);
    return (begin < str.end()) ? 
        std::wstring_view(&*begin, str.end() - begin) 
        : std::wstring_view();
}

void CountLinesInFile(const std::string& file_path, LineCounts& total_counts)
{
    std::wifstream file(file_path);
    if (file.fail())
    {
        std::cerr << "Couldn't open a file with path : " << file_path << std::endl;
        return;
    }
    LineCounts counts;
    std::wstring line;
    bool in_multiline_comment = false;

    while (std::getline(file, line)) 
    {
        
        std::wstring_view stripped_line = TrimLeft(line);


        // Check for blank lines
        if (stripped_line.empty()) 
        {
            counts.blank_lines++;
            continue;
        }

        // Check for multiline comments
        if (in_multiline_comment) 
        {
            counts.comment_lines++;
            if (stripped_line.find(L"*/") != std::string::npos) 
            {
                in_multiline_comment = false;
               
            }
            continue;
        }

        if (stripped_line.find(L"/*") == 0) 
        {
            in_multiline_comment = true;
            counts.comment_lines++;
            if (stripped_line.find(L"*/") != std::string::npos) 
            {
                in_multiline_comment = false;
            }
            continue;
        }

        // Check for single-line comments
        if (stripped_line.find(L"//") == 0)
        {
            counts.comment_lines++;
            continue;
        }

        // Otherwise, it's a code line
        counts.code_lines++;
    }

    // Check for last empty line
    if (line.empty())
    {
        counts.blank_lines++;

    }
    std::lock_guard lg(g_mutex);
    total_counts += counts;
}

static inline void ProcessFile(const fs::directory_entry& entry, LineCounts& total_counts, size_t& total_files)
{
    if (entry.is_regular_file()) 
    {
        std::string extension = entry.path().extension().string();
        
        if (extension == ".h" || extension == ".hpp" || extension == ".c" || extension == ".cpp")
        {
            total_files++;
            CountLinesInFile(entry.path().string(), total_counts);
        }
        
    }
}

size_t ProcessDirectory(const std::string& root_folder, LineCounts& total_counts)
{
    try
    {
        size_t total_files = 0;
        for (const auto& entry : fs::recursive_directory_iterator(root_folder))
        {
            ProcessFile(entry, total_counts, total_files);
        }
        return total_files;
    }
    catch (std::filesystem::filesystem_error& er)
    {
        std::cerr << er.what() << std::endl;
        return 0;
    }
}

size_t ProcessDirectoryConcurrently(const std::string& root_folder, LineCounts& total_counts)
{
    size_t total_files = 0;
    try
    {
        
        std::vector<std::future<void>> tasks;

        for (const auto& entry : fs::recursive_directory_iterator(root_folder))
        {
            if (entry.is_regular_file())
            {
                std::string extension = entry.path().extension().string();

                if (extension == ".h" || extension == ".hpp" || extension == ".c" || extension == ".cpp")
                {
                    total_files++;
                    tasks.push_back(std::async(std::launch::async, CountLinesInFile,
                        entry.path().string(), std::ref(total_counts)));
                }

            }
        }

        for (auto& task : tasks) {
            task.wait();
        }
        return total_files;
    }
    catch (std::filesystem::filesystem_error& er)
    {
        std::cerr << er.what() << std::endl;
        return total_files;
    }
}

void SaveToStream(std::ostream& os, const LineCounts& lc, const size_t total_files,
                  std::chrono::duration<double> executionTime)
{
    os << "Total number of processed files: " << total_files << '\n' << lc << '\n'
        << "Execution time: " << executionTime.count() << " seconds\n";
}
    