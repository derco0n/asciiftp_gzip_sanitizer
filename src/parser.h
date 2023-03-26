#pragma once 
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <cstdint>
#include <zlib.h>

class parser {
    protected:
    std::string _inputfile;
    std::vector<uint64_t> _inputcontent;
    std::vector<uint64_t> _positions;
    bool is_valid_gzip(const std::vector<uint64_t>&);
    

    public:
    parser(std::string inputfile); //constructor
    ~parser(); //destructor

    void parse();
    bool repair();
    const std::vector<uint64_t>& getInputContent() const;
    bool write_output_file(const std::string&);

};