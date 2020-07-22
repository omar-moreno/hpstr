/**
 * @file Process.cxx
 * @brief Class which represents the process under execution.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Processing/Process.h"

Process::Process() {}

Process::Process(const Parameters& configuration) {

    input_files_ = configuration.getParameter< std::vector< std::string > >("input_files"); 
    output_files_ = configuration.getParameter< std::vector< std::string > >("output_files"); 
}

void Process::run() {

    try {
    
        int n_events_processed = 0;
        
        if (input_files_.empty()) 
            throw std::runtime_error("Please specify files to process.");

        // Create an object used to manage the input and output files.
        Event event;  

        int cfile = 0; 
        for (auto ifile : input_files_) { 
            
            std::cout << "---- [ hpstr ][ Process ]: Processing file " 
                      << ifile << std::endl;

            // Open the input file to process.  If the input file can't be 
            // opened, throw an exception.
            EventFile* file{nullptr}; 
            if (!output_files_.empty()) {
                file = new EventFile(ifile, output_files_[cfile]);
            } else {
                // Throw an exception
            } 
            file->setupEvent(&event); 

            TTree* tree = event.getTree(); 
            // first, notify everyone that we are starting
            for (auto module : sequence_) {
                module->initialize(tree);
            }

            // Process all events.
            while (file->nextEvent() && (event_limit_ < 0 || (n_events_processed < event_limit_))) {
                if (n_events_processed%1000 == 0)
                    std::cout << "---- [ hpstr ][ Process ]: Event: " << n_events_processed << std::endl;
                event.Clear(); 
                for (auto module : sequence_) { 
                    module->process(&event); 
                }
                ++n_events_processed;
            }
            ++cfile; 
        
            // Finalize all modules. 
            for (auto module : sequence_) { 
                module->finalize(); 
            }

            if (file) {
                file->close(); 
                delete file;
                file = nullptr;
            }

        }

    } catch (std::exception& e) {
        std::cerr << "---- [ hpstr ][ Process ]: Error! " << e.what() << std::endl;
    }
}

void Process::addFileToProcess(const std::string& filename) {
    input_files_.push_back(filename);
}

void Process::addOutputFileName(const std::string& output_filename) {
    output_files_.push_back(output_filename);
}

void Process::addToSequence(Processor* mod) {
    sequence_.push_back(mod);
}

