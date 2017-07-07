#include "readTSPLIB.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <cstdlib>

void readTSPLIB::initCity()
{
    CITY = (int **)malloc(sizeof(int *) * NUMBEROFCITIES);
    for (int i = 0; i < NUMBEROFCITIES; i++)
    {
        CITY[i] = (int *)malloc(sizeof(int) * 2);
    }
}

void readTSPLIB::readFile(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Error: argment must be 2. like \"./a.out *.tsp\"" << std::endl;
        exit(1);
    }
    std::string filename = argv[1];

    std::ifstream reading_file;
    reading_file.open(filename, std::ios::in);
    if(!reading_file) {
        std::cerr << "Error: cannot open file(" << filename << ")" << std::endl;
        exit(1);
    }

    std::string reading_line_buffer;

    while (!reading_file.eof())
    {
        // read by line
        std::getline(reading_file, reading_line_buffer);

        // End of file *.tsp
        if (reading_line_buffer == "EOF")
            break;

        // read by delimiter on reading "one" line
        const char delimiter = ' ';
        std::string index, arg_first, arg_second;
        std::istringstream line_separater(reading_line_buffer);
        do{
        std::getline(line_separater, index, delimiter);
        }while(index == "");
        std::getline(line_separater, arg_first, delimiter);
        std::getline(line_separater, arg_second, delimiter);

        if (index == "DIMENSION")
        {
            NUMBEROFCITIES = atoi(arg_second.c_str());
            initCity();
        }
        else if (std::all_of(index.cbegin(), index.cend(), isdigit))
        {
            CITY[atoi(index.c_str()) - 1][0] = atoi(arg_first.c_str());
            CITY[atoi(index.c_str()) - 1][1] = atoi(arg_second.c_str());
        }
    }
}