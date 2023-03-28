#include "parser.h"

//std::string _inputfile;


parser::parser(std::string inputfile, std::string workdir){ //constructor
    this->_inputfile=inputfile;
    this->_workdir=workdir;

    this->_statefile = _workdir;
        if (!this->_statefile.empty() && this->_statefile.back() == '/') {
            this->_statefile.pop_back();
            }
    this->_statefile=this->_statefile+"/state.bin";
}

parser::~parser(){ //destructor
    this->savestate();
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

void parser::savestate(){
    std::ofstream outfile(this->_statefile, std::ios::out | std::ios::binary);
    if (!outfile) {
        // Failed to open the output file, so return false
        return;
    }
    
    //check if both options had been checked at that position
    for (const uint64_t& elem : _tried_positionsboth){        
        outfile.write(reinterpret_cast<const char*>(&elem), sizeof(uint64_t));
    }

    // Check if an error occurred during the write
    if (!outfile) {
        // An error occurred during the write, so return false
        return;
    }

    // Close the output file
    outfile.close();
}

void parser::loadstate() {    
    
    // Open the input file
    std::ifstream infile(this->_statefile, std::ios::in | std::ios::binary);
    if (!infile) {
        // Failed to open the input file
        return;
    }
    
    // Determine the size of the file in bytes
    infile.seekg(0, std::ios::end);
    std::streampos fileSize = infile.tellg();
    infile.seekg(0, std::ios::beg);
    
    // Check that the file size is a multiple of 8 bytes (size of uint64_t)
    if (fileSize % sizeof(uint64_t) != 0) {
        // File size is not a multiple of 8 bytes
        return;
    }
    
    // Read the file into the vector
    this->_tried_positionsboth.resize(fileSize / sizeof(uint64_t));
    infile.read(reinterpret_cast<char*>(this->_tried_positionsboth.data()), fileSize);
    
    // Check if an error occurred during the read
    if (!infile) {
        // An error occurred during the read, so return an empty vector
        return;
    }
    
    // Close the input file
    infile.close();
    
    return;
}

bool parser::repair_mt() {
    if (is_valid_gzip(_inputcontent)){
        std::cout << "Input-data is already valid gzip. Nothing to do." << std::endl;    
        return true;
    }
    
    // Make a copy of the original input content
    //std::vector<uint8_t> repaired_content = _inputcontent;    

    std::vector<std::thread> threads;
    int num_threads = std::thread::hardware_concurrency();
    uint256_t possibilities = 1 << _positions.size(); // This is the same as 2^_positions.size()

    std::mutex mutiter;
    uint16_t curriter=0; //iteration counter used to save state every 65535 cycles
    uint256_t iterations=0; //iteration counter used to count all iterations

    std::cout << "About to check " << possibilities << " possibilities multi-threaded using " << num_threads << " Threads." << std::endl;
    int batch_size = (1 << _positions.size()) / num_threads;

    std::mutex found_valid_mutex;
    bool found_valid = false;
    for (int t = 0; t < num_threads; t++) {
        uint64_t start = t * batch_size;
        uint64_t end = (t == num_threads - 1) ? (1 << _positions.size()) : (start + batch_size);

        threads.emplace_back([&, start, end] {

            for (uint64_t i = start; i < end; i++) {                 
                bool triedxa=false;
                bool triedxd=false;

                this->_tried_positionsboth_mutex.lock();
                //Check if the current value already had been checked and skip it ...
                bool donebefore=std::find(_tried_positionsxa.begin(), _tried_positionsxa.end(), i) != _tried_positionsxa.end();
                this->_tried_positionsboth_mutex.unlock();
                if (donebefore){ //if (contains...)
                    continue;
                    }                

                

                std::vector<uint8_t> local_repaired = _inputcontent;
                
                for (uint64_t j = 0; j < _positions.size(); j++) {
                    mutiter.lock();
                    if (iterations < uint256_max-1){
                        iterations++;
                    }
                    if (curriter < UINT16_MAX-1){
                        curriter++;
                    }
                    else {                        
                        std::cout << iterations << "/" << possibilities << " possibilities tried so far..." << std::endl;
                        savestate();
                        curriter=0;
                    }
                    mutiter.unlock();
                    
                    if (i & (1 << j)) {
                        // Replace 0x0a with 0x0d at this position
                        if (local_repaired[_positions[j]] == 0x0A) {
                            local_repaired[_positions[j]] = uint8_t(0x0D);
                            triedxd=true;
                            this->_tried_positionsxd_mutex.lock();
                            _tried_positionsxd.push_back(_positions[j]); //store this position as tried.
                            this->_tried_positionsxd_mutex.unlock();
                        }
                        
                    } else {
                        local_repaired[_positions[j]] = _inputcontent[_positions[j]];
                        triedxa=true;
                        this->_tried_positionsxa_mutex.lock();
                        _tried_positionsxa.push_back(_positions[j]); //store this position as tried.
                        this->_tried_positionsxa_mutex.unlock();
                    } 
                    if (triedxa && triedxd){ //store that both positions had been tried
                        //std::cout << "both tried at " << _positions[j] << std::endl;
                        this->_tried_positionsboth_mutex.lock();
                        this->_tried_positionsboth.push_back(_positions[j]);
                        this->_tried_positionsboth_mutex.unlock();
                    }                   
                }

                if (is_valid_gzip(local_repaired)) {
                    // The repaired content is valid, so we're done
                    std::lock_guard<std::mutex> lock(found_valid_mutex);
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


