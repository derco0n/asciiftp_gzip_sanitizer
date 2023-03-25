#include "parser.h"

std::string _inputfile;


parser::parser(std::string inputfile){ //constructor
    this->_inputfile=inputfile;
}

parser::~parser(){ //destructor

}

void parser::parse(){
    std::cout << "Parsing..." << std::endl;
    std::ifstream file(this->_inputfile, std::ios::binary);
    if (!file)
    {
        std::cerr << "Error opening file " << this->_inputfile << std::endl;
        return;
    }
    
    uint64_t pos = 0;
    uint8_t byte;
    while (file.read(reinterpret_cast<char*>(&byte), sizeof(byte)))
    {
        if (byte == 0x0a)
        {
            this->_positions.push_back(pos);
            std::cout << pos << std::endl;
        }
        pos++;
    }

    file.close();


    return;
}

