#pragma once 
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <cstdint>

class parser {
    protected:
    std::string _inputfile;
    std::vector<uint64_t> _positions;

    public:
    parser(std::string inputfile); //constructor
    ~parser(); //destructor

    void parse();

};