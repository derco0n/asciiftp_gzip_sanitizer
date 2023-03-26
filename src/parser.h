#pragma once 
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <cstdint>
#include <zlib.h>
#include <cmath>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>

class parser {
    protected:
    std::string _inputfile;
    std::vector<uint8_t> _inputcontent;
    std::vector<uint64_t> _positions;
 
    public:
    parser(std::string inputfile); //constructor
    ~parser(); //destructor

    bool parse();
    bool repair();
    bool repair_mt();
    const std::vector<uint8_t>& getInputContent() const;
    bool write_output_file(const std::string&);
    bool is_valid_gzip(const std::vector<uint8_t>&);
};