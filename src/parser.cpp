#include "parser.h"

//std::string _inputfile;


parser::parser(std::string inputfile){ //constructor
    this->_inputfile=inputfile;
}

parser::~parser(){ //destructor

}

/**
 * Checks whether the given data is a valid and complete gzip file.
 * 
 * @param data A vector containing the binary data to check
 * @return True if the data is a valid and complete gzip file, false otherwise
 */
bool parser::is_valid_gzip(const std::vector<uint64_t>& data) {
    // Create a z_stream for decompression
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = 0;
    stream.next_in = Z_NULL;

    // Initialize the z_stream for gzip decompression
    int ret = inflateInit2(&stream, 16 + MAX_WBITS);
    if (ret != Z_OK) {
        return false;
    }

    // Set the input buffer to the data vector
    stream.avail_in = data.size() * sizeof(uint64_t);
    stream.next_in = reinterpret_cast<Bytef*>(const_cast<uint64_t*>(data.data()));

    // Create an output buffer for decompressed data
    std::vector<uint8_t> outbuf(1024 * 1024);

    // Decompress the input data
    do {
        // Set the output buffer for decompressed data
        stream.avail_out = outbuf.size();
        stream.next_out = outbuf.data();

        // Decompress the input data into the output buffer
        ret = inflate(&stream, Z_NO_FLUSH);
        if (ret < 0) {
            // An error occurred during decompression, so clean up and return false
            inflateEnd(&stream);
            return false;
        }
    } while (stream.avail_out == 0);

    // Clean up the z_stream
    inflateEnd(&stream);

    // Return true if the decompressed data ended with the end-of-stream (EOS) flag
    return (ret == Z_STREAM_END);
}

bool parser::repair() {
    // Make a copy of the original input content
    std::vector<uint64_t> repaired_content = _inputcontent;

    // Try every possible combination of replacing 0x0a with 0x0d
    for (uint64_t i = 0; i < (1 << _positions.size()); i++) {
        for (uint64_t j = 0; j < _positions.size(); j++) {
            if (i & (1 << j)) {
                // Replace 0x0a with 0x0d at this position
                repaired_content[_positions[j]] = 0x0d;
            } else {
                // Keep the original byte at this position
                repaired_content[_positions[j]] = _inputcontent[_positions[j]];
            }
        }

        // Test if the repaired content is a valid gzip file
        if (is_valid_gzip(repaired_content)) {
            // The repaired content is valid, so we're done
            _inputcontent = repaired_content;
            return true;
        }
    }

    // If we get here, we couldn't repair the file
    //throw std::runtime_error("Failed to repair gzip file");
    return false;
}

// Public accessor function for _inputcontent
const std::vector<uint64_t>& parser::getInputContent() const {
    return this->_inputcontent;
}

void parser::parse(){
    /*
    In my special case it looks like something went horribly wrong.
    The file doesn't contain any 0x0d-byte anymore.
    Will have to dig deeper, to find out what happened during transfer.
    */
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
        this->_inputcontent.push_back(byte); //read every byte of the input file into _inputcontent
        if (byte == 0x0a) //if the byte found is a 0x0a...
        {
            this->_positions.push_back(pos); //... store its position in _positions            
        }
        pos++;
    }
    file.close();   

    return;
}

/**
 * Writes the repaired binary data to a file.
 * 
 * @param outputfile The name of the output file to write to * 
 * @return True if the write was successful, false otherwise
 */
bool parser::write_output_file(const std::string& outputfile/*, const std::vector<uint64_t>& data*/) {
    // Open the output file for writing in binary mode

    std::ofstream outfile(outputfile, std::ios::out | std::ios::binary);
    if (!outfile) {
        // Failed to open the output file, so return false
        return false;
    }

    // Write the repaired binary data to the output file
    outfile.write(reinterpret_cast<const char*>(this->_inputcontent.data()), this->_inputcontent.size() * sizeof(uint64_t));

    // Check if an error occurred during the write
    if (!outfile) {
        // An error occurred during the write, so return false
        return false;
    }

    // Close the output file
    outfile.close();

    // Return true to indicate success
    return true;
}


