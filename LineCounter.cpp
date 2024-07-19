#include "LineCounter.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <future>  



static std::wstring_view TrimLeft(const std::wstring& str) {
    auto begin = std::find_if_not(str.begin(), str.end(), ::isspace);
    return (begin < str.end()) ?
        std::wstring_view(&*begin, str.end() - begin)
        : std::wstring_view();
}



void LineCounter::CountLinesInFile(const std::string& file_path)
{
    std::wifstream file(file_path);
    if (file.fail())
    {
        std::cerr << "Couldn't open a file with path : " << file_path << std::endl;
        return;
    }

    LineCountStatistic counts;
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
    std::lock_guard lg(m_mutex);
    m_total_line_counts += counts;
}

bool LineCounter::CheckForSingleFile(const std::string& root_folder)
{
    if (!std::filesystem::is_directory(root_folder))
    {
        ProcessFile(std::filesystem::directory_entry(root_folder));
        return false;
    }
    return true;
}

bool LineCounter::IsCppOrCFile(const std::string& extension)
{
    return extension == ".h" || extension == ".hpp" || extension == ".c" || extension == ".cpp";
}

void LineCounter::ProcessFile(const std::filesystem::directory_entry& entry)
{
    if (entry.is_regular_file())
    {
        std::string extension = entry.path().extension().string();

        if (IsCppOrCFile(extension))
        {
            m_total_files++;
            CountLinesInFile(entry.path().string());
        }

    }
}

void LineCounter::ProcessDirectory(const std::string& root_folder)
{
    if (!CheckForSingleFile(root_folder))
    {
        return;
    }
    try
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(root_folder))
        {
            ProcessFile(entry);
        }
    }
    catch (std::filesystem::filesystem_error& er)
    {
        std::cerr << er.what() << std::endl;
    }
}

void LineCounter::ProcessFileConcurrently(const std::filesystem::directory_entry& entry, 
                                          std::vector<std::future<void>>& tasks)
{
    if (entry.is_regular_file())
    {
        std::string extension = entry.path().extension().string();

        if (IsCppOrCFile(extension))
        {
            m_total_files++;
            tasks.push_back(std::async(std::launch::async, &LineCounter::CountLinesInFile, this,
                entry.path().string()));
        }

    }
}

void LineCounter::ProcessDirectoryConcurrently(const std::string& root_folder)
{
    if (!CheckForSingleFile(root_folder))
    {
        return;
    }
    try
    {
        std::vector<std::future<void>> tasks;

        for (const auto& entry : std::filesystem::recursive_directory_iterator(root_folder))
        {
            ProcessFileConcurrently(entry, tasks);
        }

        for (auto& task : tasks) {
            task.wait();
        }
    }
    catch (std::filesystem::filesystem_error& er)
    {
        std::cerr << er.what() << std::endl;
    }
}

void LineCounter::SaveToStream(std::ostream& os, std::chrono::duration<double> executionTime)
{
    os << "Total number of processed files: " << m_total_files << '\n' << m_total_line_counts
    << '\n'
        << "Execution time: " << executionTime.count() << " seconds\n";
}