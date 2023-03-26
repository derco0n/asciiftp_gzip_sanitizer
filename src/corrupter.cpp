/*
This will artificially corrupt a binary file in the same way, as it happened in my use case.
This program can be used to test the effectiveness of the repair programm.

This can seriously damage your files.
USE WITH CAUTION. ONLY RUN AGAINST TEST-DATA! ALWAYS MAKE BACKUPS!
If you don't know what you're doing: Don't execute!
*/

#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>

using namespace std;

int main(int argc, char* argv[]) {
    int opt;
    string inputFilename, outputFilename;

    // Parse command-line options
    while ((opt = getopt(argc, argv, "i:o:")) != -1) {
        switch (opt) {
            case 'i':
                inputFilename = optarg;
                break;
            case 'o':
                outputFilename = optarg;
                break;
            default:
                cerr << "Usage: " << argv[0] << " -i <input_file> -o <output_file>" << endl;
                return 1;
        }
    }

    // Check if input and output filenames are provided
    if (inputFilename.empty() || outputFilename.empty()) {
        cerr << "Usage: " << argv[0] << " -i <input_file> -o <output_file>" << endl;
        return 1;
    }

    // Warn the user.
    std::cout << "WARNING: This will read data from \"" << inputFilename << "\", replaces all 0x0D-Byte with 0x0A, and writes the modified data to \"" << outputFilename << "\". " << std::endl << "Press Crtl+C to abort or any other key to continue..." << std::endl;
    getchar();

    // Open input file
    ifstream inputFile(inputFilename, ios::binary);
    if (!inputFile) {
        cerr << "Error opening input file: " << inputFilename << endl;
        return 1;
    }

    // Open output file
    ofstream outputFile(outputFilename, ios::binary);
    if (!outputFile) {
        cerr << "Error opening output file: " << outputFilename << endl;
        return 1;
    }

    // Read input file and replace 0x0D with 0x0A
    char c;
    while (inputFile.get(c)) {
        if (c == 0x0D) {
            c = 0x0A;
        }
        outputFile.put(c);
    }

    // Close files
    inputFile.close();
    outputFile.close();

    return 0;
}