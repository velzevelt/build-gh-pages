#include <filesystem>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include "trim.cpp"

#ifdef WIN32
#include <windows.h>
#include <processenv.h>
#include <shellapi.h>
#endif

namespace fs = std::filesystem;

#define USAGE "Usage: build-gh-pages project_dir output_dir --root-prefix \"/project-name/\""
#define ROOT_PREFIX_L "--root-prefix"
#define REPLACE_IN_L "--replace-in"
#define NO_COPY_L "--no-copy"
#define MIN_ARGC 4


template<typename T>
int exit_on_not_exists(T arg)
{
    if (not fs::exists(arg))
    {
        #ifdef WIN32
        std::wcerr << "Invalid input provided, file \"" << arg << "\" not found" << '\n'
                    << USAGE << '\n';
        #else
        std::cerr << "Invalid input provided, file \"" << arg << "\" not found" << '\n'
                    << USAGE << '\n';
        #endif
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}   

int exit_on_not_directory(const fs::path &p)
{
    if (not fs::is_directory(p))
    {
        std::cout << "Invalid input provided, not a directory " << p << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

template<typename T>
int exit_on_empty(T &s)
{
    if (s.empty())
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

struct Args
{
    fs::path input_dir;
    fs::path output_dir;
    std::string root_prefix;
    std::vector<std::string> replace_in_extensions;

    bool force_rebuild = false;
    bool no_copy = false;
};


bool is_short_opt(const std::string &src)
{
    try {return src.at(0) == '-';}
    catch (std::out_of_range &e) { return false;}
}

bool is_long_opt(const std::string &src)
{
    try {return src.at(0) == '-' and src.at(1) == '-';}
    catch (std::out_of_range &e) { return false;}
}

std::string str_tolower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), 
            [](unsigned char c){ return std::tolower(c); }
            );
    return s;
}

std::vector<std::size_t> str_find_all(const std::string &haystack, const std::string &needle)
{
    std::vector<std::size_t> positions;
    size_t pos = haystack.find(needle, 0);
    while(pos != std::string::npos)
    {
        positions.push_back(pos);
        pos = haystack.find(haystack, pos + 1);
    }

    return positions;
}

bool is_answer_yes(std::string s)
{
    if (s.empty())
    {
        return false;
    }

    trim(s);
    s = str_tolower(s);
    return s.compare("y") == 0 or s.compare("yes") == 0;
}

int parse_args(int argc, char **argv, Args &res)
{
    if (argc - 1 < MIN_ARGC)
    {
        std::cout << "Expected at least " << MIN_ARGC << " arguments got " << argc - 1 << "\n";
        std::cout << USAGE << "\n";
        return EXIT_FAILURE;
    }
    
    res.input_dir = argv[1];
    res.output_dir = argv[2];

    std::string last_option;
    for (int i = 3; i < argc; i++)
    {
        std::string arg = argv[i];
        bool is_option = is_long_opt(arg) or is_short_opt(arg);
        if (is_option)
        {
            last_option = arg;

            if (last_option.compare(NO_COPY_L) == 0)
            {
                res.no_copy = true;
            }

            continue;
        }

        if (last_option.compare(ROOT_PREFIX_L) == 0)
        {
            res.root_prefix = arg;
        }
        else if (last_option.compare(REPLACE_IN_L) == 0)
        {
            res.replace_in_extensions.push_back("." + arg);
        }
        
    }

    return EXIT_SUCCESS;
}

int check_args(const Args &args)
{
    int res = exit_on_not_directory(args.input_dir)
            + exit_on_empty(args.root_prefix)
            ;

    return res;
}

void print_args(const Args &args)
{
    std::cout << "Input dir: " 
              << args.input_dir 
              << "\nOut dir: " 
              << args.output_dir 
              << "\nPrefix: "
              << args.root_prefix
              ;
    
    if (not args.replace_in_extensions.empty())
    {
        std::cout << "\nExs: ";
        for (auto ext : args.replace_in_extensions)
        {
            std::cout << ext << ' ';
        }
    }

    std::cout << "\n";
}

int main(int argc, char **argv)
{
    Args args;
    int failure = parse_args(argc, argv, args);
    if (failure != EXIT_SUCCESS)
        return failure;

    print_args(args);

    failure = check_args(args);
    if (failure != EXIT_SUCCESS)
        return failure;


    if (fs::exists(args.output_dir))
    {
        std::cout << "Output directory already exists\n";
        bool make_copy = args.no_copy == false;
        if (make_copy)
        {
            std::cout << "Removing dir " << args.output_dir << '\n';
            fs::remove_all(args.output_dir);
            std::cout << "Copying dir " << args.input_dir
                    << " as " << args.output_dir
                    << '\n';
            fs::copy(args.input_dir, args.output_dir);
        }
        else
        {
            std::cout << "No copy created since used " << NO_COPY_L << '\n';
            args.output_dir = args.input_dir;
        }
    }
    else
    {
            std::cout << "Copying dir " << args.input_dir
                    << " as " << args.output_dir
                    << '\n';
            fs::copy(args.input_dir, args.output_dir);
    }


    std::vector<fs::path> files;
    for (auto const &dir_entry : fs::recursive_directory_iterator(args.output_dir))
    {
        fs::path p = dir_entry.path();

        if (args.replace_in_extensions.empty()
            or
            std::find(args.replace_in_extensions.begin(), 
            args.replace_in_extensions.end(), 
            p.extension()) != args.replace_in_extensions.end()
        )
        {
            files.push_back(p);
        }
    }

    for (auto &p : files)
    {
        std::cout << "Scanning file " << p << '\n';
        
        std::ifstream in_f(p);
        if (not in_f.is_open())
        {
            std::cerr << "Cannot open file " << p << '\n';
            continue;
        }

        fs::path temp_path = p;
        temp_path.replace_extension(".temp");
        
        std::ofstream out_f(temp_path);
        if (not out_f.is_open())
        {
            std::cerr << "Cannot create temp file " << temp_path << '\n';
            in_f.close();
            continue;
        } 

        std::size_t replace_count = 0;
        std::string line;
        while (std::getline(in_f, line))
        {
            auto root_positions = str_find_all(line, "/");
            for (auto pos : root_positions)
            {
                std::size_t prev_pos = pos - 1;
                char prev_char = '(';
                try { prev_char = line.at(prev_pos);}
                catch (std::out_of_range &e) { prev_char = '(';} // '/' is the first symbol in file, still valid
                
                while (isspace(prev_char))
                {
                    if (prev_pos == 0)
                    {
                        prev_char = '(';
                        break;
                    }

                    prev_char = line[prev_pos];
                    prev_pos--;
                }

                bool valid_root = prev_char == '(' or prev_char == '\'' or prev_char == '\"';
                if (valid_root)
                {
                    line.replace(pos, 1, args.root_prefix);
                    replace_count++;
                }
            }

            out_f << line << "\n";
        }

        in_f.close();
        out_f.close();

        std::cout << "Scanning done, replaced " << replace_count << " symbols" << '\n';
        
        fs::remove(p);
        fs::copy(temp_path, p);
        fs::remove(temp_path);
    }

    return EXIT_SUCCESS;
}
