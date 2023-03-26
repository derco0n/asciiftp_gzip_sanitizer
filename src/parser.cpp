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
bool parser::is_valid_gzip(const std::vector<uint8_t>& data) {
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
    stream.avail_in = data.size() * sizeof(uint8_t);
    stream.next_in = reinterpret_cast<Bytef*>(const_cast<uint8_t*>(data.data()));

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
    if (is_valid_gzip(_inputcontent)){
        std::cout << "Input-data is already valid gzip. Nothing to do." << std::endl;    
        return true;
    }

    // Make a copy of the original input content
    std::vector<uint8_t> repaired_content = _inputcontent;

    std::cout << "Checking data single-threaded." << std::endl;

    // Try every possible combination of replacing 0x0a with 0x0d
    /*
    i: used to iterate through all possible binary values up to the size of the replacement positions
    j: used to iterate through each replacement position and perform the replacement operation if necessary
    (1 << _positions.size()):   This expression generates a bit mask with a binary value of 1 shifted to the left by the number of replacement positions.
                                The result is a binary value with a 1 in the most significant bit and 0s in all other bits,
                                which is equivalent to the decimal value 2 raised to the power of the number of replacement positions.
                                This expression is used as the limit of the outer loop to iterate through every possible binary value up to this limit.


    The code is trying to repair a corrupted file by replacing some bytes with a different value.
    It does this by trying every possible combination of replacement values for a specific set of byte positions.
    The outer loop iterates through all possible binary values up to a limit, 
    which is calculated by shifting the value 1 to the left by the number of byte positions. 
    The inner loop iterates through each byte position and determines whether to replace the byte with a different value or keep the original byte, 
    based on whether the corresponding bit in the current binary value is set.
    If the resulting repaired content is valid, the function returns true and the original content is replaced with the repaired content.
    If no valid repaired content is found, the function returns false.
    */

    for (uint64_t i = 0; i < (1 << _positions.size()); i++) { 
        for (uint64_t j = 0; j < _positions.size(); j++) {
            if (i & (1 << j)) {
                // Replace 0x0a with 0x0d at this position
                if (repaired_content[_positions[j]]==0x0A){
                    /*
                    std::cout << "Altering \"";
                    std::cout << std::showbase << std::hex << static_cast<int>(repaired_content[_positions[j]]);
                    std::cout << "\" to \"0x0D\" at position ";
                    std::cout << std::showbase << std::hex << static_cast<int>(j) << std::endl;
                    */
                    repaired_content[_positions[j]] = uint8_t(0x0D);
                }
                
            } else {
                // Keep the original byte at this position
                /*
                std::cout << "Keeping \"";
                std::cout << std::showbase << std::hex << static_cast<int>(_inputcontent[_positions[j]]);
                std::cout << "\" at position ";
                std::cout << std::showbase << std::hex << static_cast<int>(j) << std::endl;                
                */
                repaired_content[_positions[j]] = _inputcontent[_positions[j]];
            }

            // Test if the repaired content is a valid gzip file
            if (is_valid_gzip(repaired_content)) {
                // The repaired content is valid, so we're done
                _inputcontent = repaired_content;
                return true;
            }
            /*
            else {
                std::cout << "Not a valid gzip so far..." << std::endl;
            }
            */
        }
    }

    // If we get here, we couldn't repair the file
    //throw std::runtime_error("Failed to repair gzip file");
    return false;
}

bool parser::repair_mt() {
    if (is_valid_gzip(_inputcontent)){
        std::cout << "Input-data is already valid gzip. Nothing to do." << std::endl;    
        return true;
    }
    
    // Make a copy of the original input content
    std::vector<uint8_t> repaired_content = _inputcontent;    

    std::vector<std::thread> threads;
    int num_threads = std::thread::hardware_concurrency();
    std::cout << "Checking data multi-threaded using " << num_threads << " Threads." << std::endl;
    int batch_size = (1 << _positions.size()) / num_threads;

    bool found_valid = false;
    for (int t = 0; t < num_threads; t++) {
        uint64_t start = t * batch_size;
        uint64_t end = (t == num_threads - 1) ? (1 << _positions.size()) : (start + batch_size);

        threads.emplace_back([&, start, end] {
            for (uint64_t i = start; i < end; i++) { 
                std::vector<uint8_t> local_repaired = repaired_content;
                for (uint64_t j = 0; j < _positions.size(); j++) {
                    if (i & (1 << j)) {
                        // Replace 0x0a with 0x0d at this position
                        if (local_repaired[_positions[j]] == 0x0A) {
                            local_repaired[_positions[j]] = uint8_t(0x0D);
                        }
                    } else {
                        local_repaired[_positions[j]] = _inputcontent[_positions[j]];
                    }
                }

                if (is_valid_gzip(local_repaired)) {
                    // The repaired content is valid, so we're done
                    _inputcontent = local_repaired;
                    found_valid = true;
                    break;
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return found_valid;
}


// Public accessor function for _inputcontent
const std::vector<uint8_t>& parser::getInputContent() const {
    return this->_inputcontent;
}

bool parser::parse(){
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
        return false;
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
    std::cout << "Found " << this->_positions.size() << " occurances of 0xA in the input file..." << std::endl;

    return true;
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
    outfile.write(reinterpret_cast<const char*>(this->_inputcontent.data()), this->_inputcontent.size() * sizeof(uint8_t));

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


