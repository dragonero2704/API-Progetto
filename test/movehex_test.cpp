#include <iostream>
#include <fstream>
#include <cassert>
#include <cstring>

#define BLOCKSIZE 4096

typedef char STATUS;

int test(std::string input, std::string output)
{
    const std::string command = "..\\movehex.exe < " + input + " > output.txt";
    // std::cout<<command<<std::endl;
    int ex_res = std::system(command.c_str());
    if (ex_res)
        return -5;
    // confrontare file output.txt e output file fornito
    std::ifstream expected, out;
    out.open("output.txt");
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

    if(!std::equal(
        std::istreambuf_iterator<char>(out.rdbuf()),
        std::istreambuf_iterator<char>(),
        std::istreambuf_iterator<char>(expected.rdbuf()))) return -1;

    out.close();
    expected.close();
    return 0;
}

int main(int argc, char **argv)
{
    assert(argc == 4);
    int result = test(std::string(argv[1]), std::string(argv[2]));
    return result;
}