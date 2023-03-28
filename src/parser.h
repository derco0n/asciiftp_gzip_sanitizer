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
#include <algorithm>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "./uint256_t/uint256_t.h"

class parser {
    protected:
    std::string _inputfile, _workdir, _statefile;    
    std::vector<uint8_t> _inputcontent;
    std::vector<uint64_t> _positions;
    std::vector<uint64_t> _tried_positionsxd; //holds information about positions that already had been tried with 0x0d
    std::vector<uint64_t> _tried_positionsxa; //holds information about positions that already had been tried with 0x0a
    std::vector<uint64_t> _tried_positionsboth; //holds information about positions where both options had been tried
    std::mutex _tried_positionsxd_mutex;
    std::mutex _tried_positionsxa_mutex;
    std::mutex _tried_positionsboth_mutex;
    void savestate();
    void loadstate();
 
    public:
    parser(std::string inputfile, std::string workdir); //constructor
    ~parser(); //destructor

    bool parse();
    bool repair();
    bool repair_mt();
    const std::vector<uint8_t>& getInputContent() const;
    bool write_output_file(const std::string&);
    bool is_valid_gzip(const std::vector<uint8_t>&);
};