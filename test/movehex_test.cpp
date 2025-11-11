#include <iostream>
#include <filesystem>
#include <fstream>
#include <cassert>
#include <cstring>

#define BLOCKSIZE 4096

typedef char STATUS;

int test(const std::filesystem::path& executable_path, const std::filesystem::path& input_path, const std::filesystem::path& expected_output_path)
{
    const auto output_filename = input_path.stem().concat(".txt.actual");
    const std::string command = executable_path.string() + " < " + input_path.string() + " > " + output_filename.string();
    // std::cout<<command<<std::endl;
    if (std::system(command.c_str()))
        return -5;
    // confrontare file output.txt e output file fornito
    std::ifstream expected, out;
    out.open(output_filename, std::ios_base::in);
    expected.open(expected_output_path);
    if (out.fail())
    {
        std::cerr << "Failed to open "<< output_filename << std::endl;
        return -3;
    }
    if (expected.fail())
    {
        std::cerr << "Failed to open \'" << expected_output_path.string() << "\'" << std::endl;
        return -4;
    }

    auto size_out = out.seekg(std::fstream::beg).tellg();
    auto size_expected = expected.seekg(std::fstream::beg).tellg();
    if (size_out != size_expected)
    {
        return -1;
    }
    out.seekg(std::fstream::beg);
    expected.seekg(std::fstream::beg);

    if (!std::equal(
            std::istreambuf_iterator<char>(out.rdbuf()),
            std::istreambuf_iterator<char>(),
            std::istreambuf_iterator<char>(expected.rdbuf())))
    {
        out.close();
        expected.close();
        return -1;
    }

    out.close();
    expected.close();
    std::remove(output_filename.string().c_str());
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>" << std::endl;
        return -3;
    }
    const auto executable_file_path = std::filesystem::path(argv[1]);
    const auto input_file_path = std::filesystem::path(argv[2]);
    const auto expected_result_path = std::filesystem::path(argv[3]);

    // check existence
    if(!std::filesystem::exists(executable_file_path))
    {
        std::cout << executable_file_path << " does not exists";
        return -1;
    }
    if(!std::filesystem::exists(input_file_path))
    {
        std::cout << input_file_path << " does not exists\n";
        return -2;
    }
    if(!std::filesystem::exists(expected_result_path))
    {
        std::cout << expected_result_path << " does not exists\n";
        return -3;
    }
    
    return test(executable_file_path, input_file_path, expected_result_path);
}