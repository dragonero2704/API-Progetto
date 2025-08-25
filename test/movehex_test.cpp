#include <iostream>
#include <fstream>
#include <cassert>
#include <cstring>

#define BLOCKSIZE 4096

#if defined(_WIN32) || defined(_WIN64)
#define EXECUTABLENAME "..\\movehex.exe < "
#else
#define EXECUTABLENAME "../movehex < "
#endif

typedef char STATUS;

int test(std::string input, std::string output)
{
    const std::string command = EXECUTABLENAME + input + " > output.txt";
    // std::cout<<command<<std::endl;
    int ex_res = std::system(command.c_str());
    if (ex_res)
        return -5;
    // confrontare file output.txt e output file fornito
    std::ifstream expected, out;
    const std::string candidateFilename = input.substr(input.find_last_of('/')+1) + ".output";
    std::rename("output.txt", candidateFilename.c_str());
    out.open(candidateFilename);
    expected.open(output);

    if (out.fail())
    {
        std::cerr << "Failed to open \'output.txt\'" << std::endl;
        return -3;
    }
    if (expected.fail())
    {
        std::cerr << "Failed to open \'" << output << "\'" << std::endl;
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
    std::remove(candidateFilename.c_str());
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>" << std::endl;
        return -2;
    }
    return test(std::string(argv[1]), std::string(argv[2]));
}