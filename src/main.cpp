/*
This program uses C++17 functionality. You may want to use e.g. g++-10 or newer and set the c++-standard to use
Example: g++-10 -std=c++17 ./main.cpp parser.cpp -lz
Or just use the Makefile
*/

#include <stdio.h>
#include <iostream>
#include <filesystem>
#include <unistd.h>
#include "parser.h"

void printhelp(std::string self){
    std::cout << "Usage: " << self << " [-h] [-i input_file] [-w working-dir]" << std::endl;
}

int main (int argc, char *argv[]){
    int opt;
    std::string workdir="/tmp/afgs/";
    std::string inputfile;
    while ((opt = getopt(argc, argv, "hi:w:")) != -1){
        switch (opt) {
            case 'h':
                printhelp(argv[0]);
                return 0;
            case 'i':                
                inputfile = optarg;
                break;
            case 'w':
                workdir = optarg;
                break;
            default:
                std::cerr << "Unknown option: " << char(optopt) << std::endl;
                return 1;
        }
    }

    if (inputfile.empty()){
        std::cerr << "Inputfile not specified" << std::endl;
        printhelp(argv[0]);
        return 1;
    }

    std::cout << "Damaged input file: " << inputfile << std::endl;
    std::cout << "Working directory: " << workdir << std::endl;

    // Check inputfile
    if (!std::filesystem::exists(inputfile)) { // check if the file exists        
        std::cout << "Inputfile  \"" << inputfile << "\" doesn't exist or isn't accessible. Aborting!" << std::endl;
    }

    // Check / create working directory
    if (!std::filesystem::exists(workdir)) { // check if the directory exists
        std::filesystem::create_directory(workdir); // create the directory if it does not exist
        std::cout << "Directory created: " << workdir << std::endl;
    }
   
    parser* fileparserPtr = new parser(inputfile, workdir);
    if (!fileparserPtr->parse()){
        std::cerr << "Unable to read inputfile. Aborting!" << std::endl;
        return 1;
    }
    std::cout << "File has been read..." << std::endl;    
    
    //if (fileparserPtr->repair()){
    if (fileparserPtr->repair_mt()){
        std::cout << "Found a possible candidate." << std::endl;
        //Inputdata could be repaired
        std::string outfile = workdir;
        if (!outfile.empty() && outfile.back() == '/') {
            outfile.pop_back();
            }
        outfile=outfile+"/candidate.gz";

        std::cout << "Please wait while repaired data is being written to \""+outfile+"\"." << std::endl;

        if (fileparserPtr->write_output_file(outfile)){
            std::cout << "repaired data has been written to \""+outfile+"\"." << std::endl;
        }
        else {
            std::cerr << "Unable to write data to \""+outfile+"\"." << std::endl;
        }
    }
    else {
        std::cout << "Unable to find a solution...Sorry." << std::endl;    
    }
    delete fileparserPtr;   

    return 0;
}