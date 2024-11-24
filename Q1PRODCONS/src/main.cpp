#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include <queue>
#include <random>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <iomanip>
#include <unordered_map>
#include <filesystem>

namespace fs = std::filesystem;

// This structure stores the details of a sales record.
struct SalesRecord {
    int day, month, year;
    int storeID, registerNum;
    float saleAmount;
};

// This class implements a shared buffer that stores sales records.
class SharedBuffer {
private:
    std::queue<SalesRecord> buffer;
    size_t maxSize;
    std::mutex mtx;
    std::condition_variable cvFull, cvEmpty;

public:
    SharedBuffer(size_t size) : maxSize(size) {}

    void produce(const SalesRecord &record) {
        std::unique_lock<std::mutex> lock(mtx);
        cvFull.wait(lock, [this]() { return buffer.size() < maxSize; });
        buffer.push(record);
        cvEmpty.notify_one();
    }

    SalesRecord consume(bool &terminate) {
        std::unique_lock<std::mutex> lock(mtx);
        cvEmpty.wait(lock, [this, &terminate]() { return !buffer.empty() || terminate; });

        if (terminate && buffer.empty()) {
            return {}; // Return a default record if no more items are available
        }

        SalesRecord record = buffer.front();
        buffer.pop();
        cvFull.notify_one();
        return record;
    }

    bool isEmpty() {
        std::lock_guard<std::mutex> lock(mtx);
        return buffer.empty();
    }

    void notifyAllDone() {
        std::unique_lock<std::mutex> lock(mtx);
        cvEmpty.notify_all(); // Notify all waiting consumers
    }
};

// Global variables
std::atomic<int> totalRecords(0);
std::atomic<bool> done(false); // Flag to indicate when all producers are finished
std::mutex statsMtx;
float globalSales = 0;
std::unordered_map<int, float> storeSales;
std::unordered_map<int, float> monthSales;

SalesRecord generateRecord(int storeID) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dayDist(1, 30);
    std::uniform_int_distribution<int> monthDist(1, 12);
    std::uniform_int_distribution<int> regDist(1, 6);
    std::uniform_real_distribution<float> amountDist(0.50, 999.99);

    SalesRecord record;
    record.day = dayDist(rng);
    record.month = monthDist(rng);
    record.year = 16;
    record.storeID = storeID;
    record.registerNum = regDist(rng);
    record.saleAmount = amountDist(rng);

    return record;
}

void producer(SharedBuffer &buffer, int storeID, int maxRecords) {
    for (int i = 0; i < maxRecords / 2; ++i) {
        SalesRecord record = generateRecord(storeID);
        buffer.produce(record);

        {
            std::lock_guard<std::mutex> lock(statsMtx);
            totalRecords++;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5 + rand() % 36));
    }

    {
        std::lock_guard<std::mutex> lock(statsMtx);
        done = true; // Signal that all producers are done
    }
    buffer.notifyAllDone(); // Notify consumers of completion
}

void consumer(SharedBuffer &buffer, int id, std::ofstream &outputFile, bool &terminate) {
    float localSales = 0;

    while (true) {
        SalesRecord record = buffer.consume(terminate);

        // Break the loop if the buffer is empty and termination is signaled
        if (terminate && record.saleAmount == 0) break;

        localSales += record.saleAmount;

        {
            std::lock_guard<std::mutex> lock(statsMtx);
            globalSales += record.saleAmount;
            storeSales[record.storeID] += record.saleAmount;
            monthSales[record.month] += record.saleAmount;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5 + rand() % 36));
    }

    {
        std::lock_guard<std::mutex> lock(statsMtx);
        outputFile << "Consumer " << id << " local sales: " << std::fixed << std::setprecision(2) << localSales << std::endl;
        std::cout << "Consumer " << id << " local sales: " << std::fixed << std::setprecision(2) << localSales << std::endl;
    }
}

void logMessage(const std::string &message, std::ofstream &outputFile) {
    std::cout << message << std::endl;
    outputFile << message << std::endl;
}

void runSimulation(int p, int c, int b, std::ofstream &outputFile) {
    SharedBuffer buffer(b);
    std::vector<std::thread> producers, consumers;
    bool terminate = false; // Local termination flag for consumers

    totalRecords = 0;
    globalSales = 0;
    storeSales.clear();
    monthSales.clear();
    done = false; // Reset the done flag for each simulation

    logMessage("Starting simulation with p=" + std::to_string(p) +
               ", c=" + std::to_string(c) + ", b=" + std::to_string(b), outputFile);

    auto startTime = std::chrono::steady_clock::now();

    logMessage("Launching producers...", outputFile);
    for (int i = 1; i <= p; ++i) {
        producers.emplace_back(producer, std::ref(buffer), i, 1000);
    }

    logMessage("Launching consumers...", outputFile);
    for (int i = 1; i <= c; ++i) {
        consumers.emplace_back(consumer, std::ref(buffer), i, std::ref(outputFile), std::ref(terminate));
    }

    for (auto &prod : producers) prod.join();

    // Signal consumers to terminate
    terminate = true;
    buffer.notifyAllDone();

    for (auto &cons : consumers) cons.join();

    auto endTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = endTime - startTime;

    logMessage("Simulation completed.", outputFile);
    logMessage("Global sales: " + std::to_string(globalSales), outputFile);
    logMessage("Elapsed time: " + std::to_string(elapsed.count()) + " seconds", outputFile);

    outputFile << "Store-wise sales:\n";
    for (const auto &store : storeSales) {
        outputFile << "  Store " << store.first << ": " << store.second << std::endl;
    }

    outputFile << "Month-wise sales:\n";
    for (const auto &month : monthSales) {
        outputFile << "  Month " << month.first << ": " << month.second << std::endl;
    }

    outputFile << "--------------------------------------\n";
}

int main() {
    std::string outputDir = "output";
    std::string outputFilePath = outputDir + "/results.txt";
    std::string timingFilePath = outputDir + "/timing_results.txt";

    if (!fs::exists(outputDir)) {
        fs::create_directories(outputDir);
    }

    std::ofstream outputFile(outputFilePath, std::ios::trunc);
    std::ofstream timingFile(timingFilePath, std::ios::trunc);

    if (!outputFile.is_open() || !timingFile.is_open()) {
        std::cerr << "Error: Could not open or create output files!\n";
        return 1;
    }

    int bufferSizes[] = {3, 10};
    int producerCounts[] = {2, 5, 10};
    int consumerCounts[] = {2, 5, 10};

    timingFile << "Buffer Size (b), Producers (p), Consumers (c), Time (seconds)\n";

    for (int b : bufferSizes) {
        for (int p : producerCounts) {
            for (int c : consumerCounts) {
                auto startTime = std::chrono::steady_clock::now();
                runSimulation(p, c, b, outputFile);
                auto endTime = std::chrono::steady_clock::now();

                std::chrono::duration<double> elapsed = endTime - startTime;

                timingFile << b << ", " << p << ", " << c << ", " << elapsed.count() << "\n";
                std::cout << "Simulation (b=" << b << ", p=" << p << ", c=" << c
                          << ") completed in " << elapsed.count() << " seconds.\n";
            }
        }
    }

    outputFile.close();
    timingFile.close();
    return 0;
}
