#ifndef _readTSPLIB_H
#define _readTSPLIB_H

class readTSPLIB {
public:
    void readFile(int argc, char** argv);

    int** CITY;
    int NUMBEROFCITIES;

private:
    void initCity();
};

#endif