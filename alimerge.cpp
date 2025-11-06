// This program is designed to look for merges of Aliquot
// sequences.  It uses the file OE_C80.txt to match the
// last occurrence of an 80 digit composite and determines
// the merged sequence, if any.  If a merge is found, the
// program determines the merge points between the two
// sequences.  This program is specifically built to work
// with base tables found at:
//    "Aliquot sequences starting on integer powers n^i"
// (http://www.aliquotes.com/aliquotes_puissances_entieres.html)
//
// It can be compiled in linux via:
//           make
/////////////////////////////////////////////////////////////////

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>

static std::map<std::string, std::string> parseC80Map();
static void checkSequence(
    const std::map<std::string, std::string> &C80Map,
    const std::string &base,
    const unsigned int exp
);
static void downloadAndOpenSequenceFile(
    std::ifstream &file,
    const std::string &fileName,
    const std::string &sequence
);
static void downloadAndOpenFile(
    std::ifstream &file,
    const std::string &fileName,
    const std::string &url,
    const std::string &fileDescription
);
static unsigned int parseUnsignedInt(const std::string &str);
static void validateUnsignedInt(const std::string &str);
static std::pair<std::string, std::string> parseElfLine(const std::string &line);
static std::string formatDuration(std::chrono::milliseconds ms);


static std::chrono::system_clock::duration downloadFileDuration;
static std::chrono::system_clock::time_point globalTimer;


int main(const int argc, const char ** const argv) {

    // Validate arguments

    if (argc < 3) {
        std::cerr << "Please invoke as: " << argv[0] << " <base> <starting exponent> [<ending exponent>]" << std::endl;
        return 1;
    }

    const std::string base = argv[1];

    validateUnsignedInt(base);

    unsigned int first = parseUnsignedInt(argv[2]);
    unsigned int last = (argc > 3) ? parseUnsignedInt(argv[3]) : first;

    if (first > last)
        std::swap(first, last);

    // Timers

    downloadFileDuration = std::chrono::system_clock::duration::zero();

    // Read C80 file

    const std::map<std::string, std::string> C80Map = parseC80Map();

    std::cout << "Running base " << base << " from " << first << " through " << last << " . . ." << std::endl;

    for (unsigned int exp = first; exp <= last; exp++) {
        checkSequence(C80Map, base, exp);
    }

    std::chrono::system_clock::time_point endTimer = std::chrono::system_clock::now();
    std::chrono::system_clock::duration totalDuration = endTimer - globalTimer;
    std::chrono::system_clock::duration computationDuration = totalDuration - downloadFileDuration;

    auto totalMs = std::chrono::duration_cast<std::chrono::milliseconds> (totalDuration);
    auto totalSec = std::chrono::duration_cast<std::chrono::seconds> (totalDuration);
    auto downloadMs = std::chrono::duration_cast<std::chrono::milliseconds>(downloadFileDuration);
    auto computeMs = std::chrono::duration_cast<std::chrono::milliseconds> (computationDuration);

    std::cout << std::endl;

    std::cout << "Total running time   : " << formatDuration(totalMs) << " (" << totalSec.count() << " seconds.)" << std::endl;
    std::cout << "Downloading file time: " << formatDuration(downloadMs) << std::endl;
    std::cout << "Computation only time: " << formatDuration(computeMs) << std::endl;

    return 0;
}

static std::map<std::string, std::string> parseC80Map() {
    std::ifstream C80File("OE_C80.txt");

    if (C80File.fail()) {
        std::string getfile;
        std::cout << "The 80 digit file was not found - download it? (y/n): ";
        std::cin >> getfile;

        globalTimer = std::chrono::system_clock::now();

        if (getfile == "y") {
            downloadAndOpenFile(C80File, "OE_C80.txt", "http://www.aliquotes.com/OE_C80.txt", "the 80 digit file");
        } else {
            std::cout << "Leaving..." << std::endl;
            std::exit(0);
        }
    } else {
        globalTimer = std::chrono::system_clock::now();
    }

    std::map<std::string, std::string> C80Map;
    std::string line;
    while (std::getline(C80File, line)) {
        auto founds = line.find(' ', 2);
        if (founds != std::string::npos) {
            std::string matchingSequence = line.substr(1, founds - 1);
            std::string composite = line.substr(founds + 1);
            C80Map[composite] = matchingSequence;
        }
    }

    C80File.close();

    return C80Map;
}

static void checkSequence(
    const std::map<std::string, std::string> &C80Map,
    const std::string &base,
    const unsigned int exp
) {
    const std::string sequence = base + "^" + std::to_string(exp);

    std::ifstream ali1, ali2;

    downloadAndOpenSequenceFile(ali1, "aliseq1", sequence);

    std::map<std::string, std::string> ali1Map;

    std::string ali1LastC80Composite;
    std::string line;
    bool foundC80 = false;

    while (std::getline(ali1, line)) {
        const auto [index, composite] = parseElfLine(line);

        ali1Map[composite] = index;

        if (composite.size() == 80) {
            ali1LastC80Composite = composite;
            foundC80 = true;
        }
    }
    ali1.close();

    if (!foundC80) {
        return;
    }

    try {
        // Throw if not found
        const std::string matchingSequence = C80Map.at(ali1LastC80Composite);
#ifdef DEBUG
        std::cout << "80 digit composite has a match in sequence " << matchingSequence << std::endl;
#endif

        downloadAndOpenSequenceFile(ali2, "aliseq2", matchingSequence);

        while (std::getline(ali2, line)) {
            const auto [index, composite] = parseElfLine(line);

            const auto ali1Search = ali1Map.find(composite);
            if (ali1Search != ali1Map.end())
            {
                std::cout << sequence + ":i" + ali1Search->second + " merges with " + matchingSequence + ":i" + index << std::endl;
                break;
            }
        }
        ali2.close();
    } catch (const std::exception&) {
#ifdef DEBUG
        std::cout << "Could not find 80 digit composite " << ali1LastC80Composite << " in OE_C80.txt" << std::endl;
#endif
    }
}

static unsigned int parseUnsignedInt(const std::string &str) {
    validateUnsignedInt(str);

    try {
        const unsigned long value = std::stoul(str);

        if (value > UINT32_MAX) {
            std::cerr << "Integer exceeds size limit!" << std::endl;
            std::exit(1);
        }

        return static_cast<unsigned int>(value);
    } catch (const std::exception&) {
        std::cerr << "Could not parse integer!" << std::endl;
        std::exit(1);
    }
}

static void validateUnsignedInt(const std::string &str) {
    if (!std::all_of(str.cbegin(), str.cend(), [](unsigned char c){ return std::isdigit(c); })) {
        std::cerr << "Value must be an unsigned integer!" << std::endl;
        std::exit(1);
    }
}

static void downloadAndOpenSequenceFile(
    std::ifstream &file,
    const std::string &fileName,
    const std::string &sequence
) {
#ifdef DEBUG
        std::cout << "Downloading sequence " << sequence;
#endif

    const std::string sequenceUrl = "https://factordb.com/elf.php?seq=" + sequence + "&type=1";
    const std::string downloadDescription = "the sequence " + sequence;
    downloadAndOpenFile(file, fileName, sequenceUrl, downloadDescription);

#ifdef DEBUG
        std::cout << " : Done" << std::endl;
#endif
}

static void downloadAndOpenFile(
    std::ifstream &file,
    const std::string &fileName,
    const std::string &url,
    const std::string &fileDescription
) {
    const std::string commandStr = R"(curl -q -s -o ')" + fileName + R"(' ')" + url + R"(')";

    const std::chrono::system_clock::time_point startTimer = std::chrono::system_clock::now();

    const int returnCode = std::system(commandStr.c_str());

    const std::chrono::system_clock::time_point endTimer = std::chrono::system_clock::now();
    downloadFileDuration += endTimer - startTimer;

    if (returnCode != 0) {
#ifdef DEBUG
        std::cout << std::endl;
#endif
        std::cerr << "Trouble has occurred while trying to download " << fileDescription << "!" << std::endl;
        std::exit(1);
    }

    file.open(fileName);

    if (!file.is_open()) {
        std::cerr << "Trouble has occurred while trying to read " << fileDescription << "!" << std::endl;
        std::exit(1);
    }
}

static std::pair<std::string, std::string> parseElfLine(const std::string &line) {
    const size_t foundp = line.find('.');
    const size_t founde = line.find('=', foundp + 4);
    const std::string index = line.substr(0, foundp - 1);
    const std::string composite = line.substr(foundp + 4, founde - (1 + foundp + 4));
    return make_pair(index, composite);
}

static std::string formatDuration(std::chrono::milliseconds ms) {
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(ms);
    ms -= std::chrono::duration_cast<std::chrono::milliseconds>(secs);
    auto mins = std::chrono::duration_cast<std::chrono::minutes>(secs);
    secs -= std::chrono::duration_cast<std::chrono::seconds>(mins);
    auto hour = std::chrono::duration_cast<std::chrono::hours>(mins);
    mins -= std::chrono::duration_cast<std::chrono::minutes>(hour);

    std::stringstream ss;
    if (hour.count())
        ss << hour.count() << " Hours : ";
    if (mins.count())
        ss << mins.count() << " Minutes : ";
    if (secs.count())
        ss << secs.count() << " Seconds : ";
    if (ms.count())
        ss << ms.count() << " Milliseconds";

    return ss.str();
}
