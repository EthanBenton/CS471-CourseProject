#include <iostream>   // Provides standard input/output stream (used for logging to console).
#include <fstream>    // Used to read from/write to files (logging results to output files).
#include <string>     // Provides the std::string class for handling text data.
#include <thread>     // Enables creation and management of multiple threads (producers and consumers).
#include <vector>     // Used to store multiple threads in a resizable array.
#include <queue>      // Provides the queue data structure for the shared buffer.
#include <random>     // Used to generate random numbers for sales records.
#include <mutex>      // Provides mutexes to ensure thread-safe access to shared resources.
#include <condition_variable> // Synchronizes threads using condition variables for producer-consumer communication.
#include <chrono>     // Used to measure time and create delays in thread execution.
#include <atomic>     // Provides atomic variables to safely share data between threads.
#include <iomanip>    // Used for precise formatting of floating-point numbers in output.
#include <unordered_map> // Provides hash maps for fast lookup of statistics (store-wise and month-wise sales).
#include <filesystem> // Used to manage file system tasks like creating directories.

namespace fs = std::filesystem; // Shortens the namespace to make filesystem operations more convenient.

// Structure to represent a sales record, containing all required fields as per the project description.
struct SalesRecord {
    int day, month, year;    // Sales date: day, month, and year.
    int storeID, registerNum; // Store ID and register number of the sales transaction.
    float saleAmount;         // The amount of the sale.
};

// Class representing a thread-safe shared buffer for storing sales records.
class SharedBuffer {
private:
    std::queue<SalesRecord> buffer;          // The buffer implemented as a queue to store records.
    size_t maxSize;                          // The maximum number of items the buffer can hold.
    std::mutex mtx;                          // Mutex to ensure thread-safe access to the buffer.
    std::condition_variable cvFull, cvEmpty; // Condition variables for producer-consumer synchronization.

public:
    // Constructor to initialize the buffer with a given size.
    SharedBuffer(size_t size) : maxSize(size) {}

    // Producer function to add a record to the buffer.
    void produce(const SalesRecord &record) {
        std::unique_lock<std::mutex> lock(mtx); // Lock the mutex to access the buffer.
        cvFull.wait(lock, [this]() { return buffer.size() < maxSize; }); // Wait if the buffer is full.
        buffer.push(record); // Add the sales record to the buffer.
        cvEmpty.notify_one(); // Notify one waiting consumer that a new item is available.
    }

    // Consumer function to retrieve a record from the buffer.
    SalesRecord consume(bool &terminate) {
        std::unique_lock<std::mutex> lock(mtx); // Lock the mutex to access the buffer.
        cvEmpty.wait(lock, [this, &terminate]() { return !buffer.empty() || terminate; }); // Wait if the buffer is empty.

        if (terminate && buffer.empty()) {
            return {}; // Return an empty record if no more items are available and termination is signaled.
        }

        SalesRecord record = buffer.front(); // Get the front record in the buffer.
        buffer.pop(); // Remove the record from the buffer.
        cvFull.notify_one(); // Notify one waiting producer that space is now available.
        return record;
    }

    // Function to check if the buffer is empty.
    bool isEmpty() {
        std::lock_guard<std::mutex> lock(mtx); // Lock the mutex for thread-safe access.
        return buffer.empty(); // Return true if the buffer is empty.
    }

    // Function to notify all waiting consumers that production is complete.
    void notifyAllDone() {
        std::unique_lock<std::mutex> lock(mtx); // Lock the mutex.
        cvEmpty.notify_all(); // Notify all consumers waiting on cvEmpty.
    }
};

// Global variables shared between threads.
std::atomic<int> totalRecords(0);    // Atomic variable to count the total number of produced records.
std::atomic<bool> done(false);      // Atomic flag to signal that all producers are finished.
std::mutex statsMtx;                // Mutex to protect access to shared statistics.
float globalSales = 0;              // Total sales across all stores and months.
std::unordered_map<int, float> storeSales; // Store-wise sales statistics.
std::unordered_map<int, float> monthSales; // Month-wise sales statistics.

// Function to generate a random sales record for a given store ID.
SalesRecord generateRecord(int storeID) {
    static std::mt19937 rng(std::random_device{}()); // Random number generator.
    std::uniform_int_distribution<int> dayDist(1, 30); // Random day (1-30).
    std::uniform_int_distribution<int> monthDist(1, 12); // Random month (1-12).
    std::uniform_int_distribution<int> regDist(1, 6); // Random register number (1-6).
    std::uniform_real_distribution<float> amountDist(0.50, 999.99); // Random sale amount (0.50-999.99).

    SalesRecord record;
    record.day = dayDist(rng); // Generate random day.
    record.month = monthDist(rng); // Generate random month.
    record.year = 16; // Fixed year.
    record.storeID = storeID; // Store ID assigned to the producer.
    record.registerNum = regDist(rng); // Generate random register number.
    record.saleAmount = amountDist(rng); // Generate random sale amount.

    return record; // Return the generated sales record.
}

// Function executed by each producer thread.
void producer(SharedBuffer &buffer, int storeID, int maxRecords) {
    for (int i = 0; i < maxRecords; ++i) { // Each producer generates a fixed number of records.
        SalesRecord record = generateRecord(storeID); // Generate a sales record for the assigned store.
        buffer.produce(record); // Add the record to the shared buffer.

        {
            std::lock_guard<std::mutex> lock(statsMtx); // Lock the mutex to safely update the total record count.
            totalRecords++; // Increment the global record count.
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5 + rand() % 36)); // Simulate processing time (5–40 ms).
    }

    done = true; // Set the global flag to indicate that all producers have completed their tasks.
    buffer.notifyAllDone(); // Notify all consumers that no more records will be produced.
}

// Function executed by each consumer thread.
void consumer(SharedBuffer &buffer, int id, std::ofstream &outputFile, bool &terminate) {
    float localSales = 0; // Local total sales for this consumer.

    while (true) {
        SalesRecord record = buffer.consume(terminate); // Retrieve a record from the buffer.

        if (terminate && record.saleAmount == 0) { // Exit if termination is signaled and the buffer is empty.
            break;
        }

        localSales += record.saleAmount; // Add the sale amount to the consumer's local total.

        {
            std::lock_guard<std::mutex> lock(statsMtx); // Lock the mutex to safely update shared statistics.
            globalSales += record.saleAmount; // Update the global sales total.
            storeSales[record.storeID] += record.saleAmount; // Update the store-wise sales total.
            monthSales[record.month] += record.saleAmount; // Update the month-wise sales total.
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5 + rand() % 36)); // Simulate processing time (5–40 ms).
    }

    // Log the local sales for this consumer to the output file.
    std::lock_guard<std::mutex> lock(statsMtx);
        outputFile << "Consumer " << id << " local sales: " << std::fixed << std::setprecision(2) << localSales << std::endl;
        std::cout << "Consumer " << id << " local sales: " << std::fixed << std::setprecision(2) << localSales << std::endl;
}

void logMessage(const std::string &message, std::ofstream &outputFile) {
    std::cout << message << std::endl;
    outputFile << message << std::endl;
}

// Function to run a single simulation with given parameters (number of producers, consumers, and buffer size).
void runSimulation(int p, int c, int b, std::ofstream &outputFile) {
    SharedBuffer buffer(b); // Create the shared buffer with the specified size.
    std::vector<std::thread> producers, consumers; // Vectors to hold producer and consumer threads.
    bool terminate = false; // Flag to indicate when consumers should terminate.

    totalRecords = 0;
    globalSales = 0;
    storeSales.clear();
    monthSales.clear();
    done = false; // Reset the done flag for each simulation

    logMessage("Starting simulation with p=" + std::to_string(p) +
               ", c=" + std::to_string(c) + ", b=" + std::to_string(b), outputFile);

    auto startTime = std::chrono::steady_clock::now(); // Record the simulation start time.

    // Launch producer threads.
    logMessage("Launching producers...", outputFile);
    for (int i = 1; i <= p; ++i) {
        producers.emplace_back(producer, std::ref(buffer), i, 1000); // Each producer generates 1000 records.
    }

    // Launch consumer threads.
    logMessage("Launching consumers...", outputFile);
    for (int i = 1; i <= c; ++i) {
        consumers.emplace_back(consumer, std::ref(buffer), i, std::ref(outputFile), std::ref(terminate));
    }

    // Wait for all producer threads to complete.
    for (auto &prod : producers) {
        prod.join();
    }

    // Signal consumers to terminate after all producers have finished.
    terminate = true;
    buffer.notifyAllDone(); // Notify all consumers that no more records will be produced.

    // Wait for all consumer threads to complete.
    for (auto &cons : consumers) {
        cons.join();
    }

    auto endTime = std::chrono::steady_clock::now(); // Record the simulation end time.
    std::chrono::duration<double> elapsed = endTime - startTime; // Calculate the elapsed simulation time.

    // Log the simulation duration to the output file.
    outputFile << "Simulation completed in " << elapsed.count() << " seconds.\n";

    // Log global statistics to the output file.
    outputFile << "Global sales: " << std::fixed << std::setprecision(2) << globalSales << std::endl;
    outputFile << "Store-wise sales:\n";
    for (const auto &[storeID, total] : storeSales) {
        outputFile << "  Store " << storeID << ": " << total << std::endl;
    }
    outputFile << "Month-wise sales:\n";
    for (const auto &[month, total] : monthSales) {
        outputFile << "  Month " << month << ": " << total << std::endl;
    }
    outputFile << "--------------------------------------\n";
}

// Main function to manage and execute all simulations.
int main() {
    // Define the directory for storing output files.
    std::string outputDir = "output";
    std::string outputFilePath = outputDir + "/results.txt";
    std::string timingFilePath = outputDir + "/timing_results.txt";

    // Ensure the output directory exists; create it if it doesn't.
    if (!fs::exists(outputDir)) {
        fs::create_directories(outputDir);
    }

    // Open the results file for writing (overwrites any existing file).
    std::ofstream outputFile(outputFilePath, std::ios::trunc);
    std::ofstream timingFile(timingFilePath, std::ios::trunc);

    if (!outputFile.is_open() || !timingFile.is_open()) {
        std::cerr << "Error: Could not open or create output files!\n";
        return 1;
    }

    // Define the parameters for the simulations.
    int bufferSizes[] = {3, 10};            // Possible buffer sizes.
    int producerCounts[] = {2, 5, 10};      // Possible numbers of producers.
    int consumerCounts[] = {2, 5, 10};      // Possible numbers of consumers.
    int totalRuns = 18; // Total number of runs (3 buffer sizes * 3 producer counts * 3 consumer counts).
    int runCounter = 1; // Initialize the run counter.

    timingFile << "Buffer Size (b), Producers (p), Consumers (c), Time (seconds)\n";

    // Run simulations for all combinations of parameters.
    for (int b : bufferSizes) {
        for (int p : producerCounts) {
            for (int c : consumerCounts) {
                // Calculate the progress percentage.
                double progress = (static_cast<double>(runCounter) / totalRuns) * 100;

                // Log run number and progress.
                std::cout << "Starting simulation " << runCounter << "/" << totalRuns
                          << " (" << std::fixed << std::setprecision(1) << progress << "% completed)" << std::endl;
                outputFile << "Starting simulation " << runCounter << "/" << totalRuns
                           << " (" << std::fixed << std::setprecision(1) << progress << "% completed)" << std::endl;

                auto startTime = std::chrono::steady_clock::now();
                runSimulation(p, c, b, outputFile);
                auto endTime = std::chrono::steady_clock::now();

                std::chrono::duration<double> elapsed = endTime - startTime;

                timingFile << b << ", " << p << ", " << c << ", " << elapsed.count() << "\n";
                std::cout << "Simulation (b=" << b << ", p=" << p << ", c=" << c
                          << ") completed in " << elapsed.count() << " seconds.\n";

                runCounter++; // Increment the run counter.
            }
        }
    }

    outputFile.close();
    timingFile.close(); // Close the results file after all simulations are complete.
    return 0; // Exit the program successfully.
}
