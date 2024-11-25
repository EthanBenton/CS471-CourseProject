#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_set>
// This program determines the effectiveness of different page replacement programs by analyzing how many page faults they each generate. 
//The program is designed to take in Byte addresses from a text file that are listed one line at a time.
//The adjustable variables are: the input file, the number of frames, the page size.
using namespace std;

// FIFO Page Replacement Algorithm
double fifoReplacement(const vector<int>& pages, int frames) {
    vector<int> memory; //This is a representation of the frames list
    double faults = 0; //used to track page faults

    for (int page : pages) { //iterate through each page look up and perform the FIFO algorithm
        if (find(memory.begin(), memory.end(), page) == memory.end()) { // Page fault
            if (memory.size() >= frames) { // if frames are full
                memory.erase(memory.begin()); // Remove oldest page
            }
            memory.push_back(page); // Add new page
            faults++; //Page fault
        }
    }
    return faults; //returns the number of page faults for this algorithm based on the selected criteria
}

// LRU Page Replacement Algorithm
double lruReplacement(const vector<int>& pages, int frames) { 
    vector<int> memory; //This is a representation of the frames list
    double faults = 0; //used to track page faults

    for (int page : pages) {
        auto it = find(memory.begin(), memory.end(), page);
        if (it != memory.end()) { // Page found
            memory.erase(it);    // Remove it to update its position
        } else {                 // Page fault
            if (memory.size() >= frames) {  // if frames are full
                memory.erase(memory.begin()); // Remove least recently used
            }
            faults++; //page fault
        }
        memory.push_back(page); // Add the page to the end
    }
    return faults; //returns the number of page faults for this algorithm based on the selected criteria
}

// MRU Page Replacement Algorithm
double mruReplacement(const vector<int>& pages, int frames) {
    vector<int> memory; //This is a representation of the frames list
    double faults = 0; //used to track page faults

    for (int page : pages) {
        auto it = find(memory.begin(), memory.end(), page);
        if (it != memory.end()) { // Page found
            memory.erase(it);    // Remove it to update its position
            memory.push_back(page); // Add the page to the end
            continue;
        } 
        else {                 // Page fault
            if (memory.size() >= frames) {  // if frames are full
                memory.pop_back(); // Remove most recently used page
            }
             memory.push_back(page); // Add the page to the end
            faults++; //used to track page faults
        }
    }
    return faults; //returns the number of page faults for this algorithm based on the selected criteria
}

// Optimal Page Replacement Algorithm
double optimalReplacement(const vector<int>& pages, int numFrames) {
    vector<int> memory; //This is a representation of the frames list
    double faults = 0;

    for (int i = 0; i < pages.size(); i++) {
        int currentPage = pages[i]; //current page will be used as the starting point of the optimal search
        auto it = find(memory.begin(), memory.end(), currentPage); //is the current page in the frames list
        if (it != memory.end()) { //page found in frames list
            continue; // No page fault
        }

        if (memory.size() < numFrames) { //if size is not full
            memory.push_back(currentPage); // Add page
            faults++; //Page fault
        } else {
            // Determine the page to replace
            int farthest = -1, replaceIndex = -1; //initialize these to invalid value to ensure proper search
            for (int j = 0; j < memory.size(); j++) { //search the frames list 
                int nextUse = -1;
                for (int k = i + 1; k < pages.size(); k++) { //search the pages for the next occurance of the page in frame J
                    if (pages[k] == memory[j]) {
                        nextUse = k; //page in frame J is next used at K time
                        break;
                    }
                }
                if (nextUse == -1) { // Page not used again
                    replaceIndex = j; //A page not used again is always the optimal replacement
                    break;
                } else if (nextUse > farthest) {
                    farthest = nextUse; //if the page J is used later than the current latest replace it with J
                    replaceIndex = j; //change the frame to replace to J
                }
            }
            memory.erase(memory.begin() + replaceIndex); // Remove the farthest page
            memory.push_back(currentPage);              // Add the current page
            faults++; //Page fault
        }
    }
    return faults; //returns the number of page faults for this algorithm based on the selected criteria
}

// Read file and store addresses
void readFile(const string& filename, vector<int>& data) {
    ifstream file(filename);

    if (file.is_open()) {
        int number;
        while (file >> number) { // Read each integer from the file
            data.push_back(number);
        }
        file.close();
    } else {
        cerr << "Could not open the file: " << filename << endl;
    }
}

// Convert addresses to pages
vector<int> makePages(const vector<int>& addresses, int pageSize) {
    vector<int> pages;
    for (int address : addresses) {
        pages.push_back(address / pageSize); // Calculate page number based on the page size
    }
    return pages;
}

// Count unique pages
int countUniquePages(const vector<int>& pages) {
    unordered_set<int> uniquePages(pages.begin(), pages.end());
    return uniquePages.size(); //counts the unique number of different pages to use as an output to the print function
}

// Print results
void print(int pageSize, int numPages, int frames, const vector<double>& faults) {
    cout << setw(10) << "Page Size" << setw(15) << "# Pages" << setw(15) << "# Frames"
         << setw(15) << "Algorithm" << setw(20) << "Fault %" << endl;

    vector<string> algorithms = {"FIFO", "LRU", "MRU", "Optimal"};
    for (int i = 0; i < faults.size(); i++) {
        cout << setw(10) << pageSize << setw(15) << numPages << setw(15) << frames
             << setw(15) << algorithms[i] << setw(20) << faults[i] << endl;
    }
}

// Run all algorithms
void runAlgorithms(vector<int>& data) {
    int frames = 12; //sets the number of memory frames 
    int pageSize = 2048; //sets the system page size
    vector<double> faults(4,0); //Initialize the vector to be size 4 (one for each algorithm) and default values of zero
    vector<int> pages = makePages(data, pageSize); //takes the Byte addresses and converts them into pages 
    int numPages = countUniquePages(pages); //finds the unique amount of pages.

    faults[0] = ((fifoReplacement(pages, frames)/data.size() * 100)); //returns how many page faults will occur from FIFO page replacement
    faults[1] = ((lruReplacement(pages, frames)/data.size() * 100));//returns how many page faults will occur from LRU page replacement
    faults[2] = ((mruReplacement(pages, frames)/data.size() * 100));//returns how many page faults will occur from MRU page replacement
    faults[3] = ((optimalReplacement(pages, frames)/data.size() * 100));//returns how many page faults will occur from optimal page replacement
    print(pageSize, numPages, frames, faults); //prints the fault information in a table format.
}

// Main function
int main() {
    vector<int> data; //A vector to store the input information
    readFile("input.txt", data); //reads in a text file (defaulted to input.txt) as a list of byte addresses and stores it
    runAlgorithms(data); // runs through the different algorthims and prints the page faults of each to the terminal.
    return 0;
}
