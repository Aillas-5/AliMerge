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
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <cstdio>
#include <cstdlib>

static std::string format_duration(std::chrono::milliseconds ms) {
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(ms);
    ms -= std::chrono::duration_cast<std::chrono::milliseconds>(secs);
    auto mins = std::chrono::duration_cast<std::chrono::minutes>(secs);
    secs -= std::chrono::duration_cast<std::chrono::seconds>(mins);
    const auto hour = std::chrono::duration_cast<std::chrono::hours>(mins);
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

static bool download(const std::string outputFile, const std::string url, std::chrono::system_clock::duration &downloadFileDuration) {
    const std::string commandStr = R"(curl -q -s -o ')" + outputFile + R"(' ')" + url + R"(')";

    const std::chrono::system_clock::time_point startTimer = std::chrono::system_clock::now();

    const int returnCode = system(commandStr.c_str());

    const std::chrono::system_clock::time_point endTimer = std::chrono::system_clock::now();
    downloadFileDuration += endTimer - startTimer;

    return (returnCode == 0);
}

int main(int argc, char** argv) {

    std::ifstream ali1, ali2;
    std::string ali1LastC80Composite, sequenceUrl;
    unsigned int first, last;
    bool downloadSucceeded;

    std::map<std::string, std::string> C80Map;
    std::map<std::string, std::string> ali1Map;

    if (argc < 3) {
        std::cerr << "Please invoke as: " << argv[0] << " <base> <starting exponent> [<ending exponent>]" << std::endl;
        return 1;
    }

    for (const char *c = argv[1]; *c; ++c) {
        if (!std::isdigit(static_cast<unsigned char>(*c))) {
            std::cerr << "Base must be an integer!" << std::endl;
            return 1;
        }
    }
    const std::string base = argv[1];

#ifdef _WIN32
    sscanf_s(argv[2], "%u", &first);
    if (argc > 3)
        sscanf_s(argv[3], "%u", &last);
    else
        last = first;
#else
    sscanf(argv[2], "%u", &first);
    if (argc > 3)
        sscanf(argv[3], "%u", &last);
    else
        last = first;
#endif

    if (first > last)
        std::swap(first, last);

    std::chrono::system_clock::duration downloadFileDuration = std::chrono::system_clock::duration::zero();
    std::chrono::system_clock::time_point globalTimer;

    std::ifstream C80File("OE_C80.txt");

    if (C80File.fail())
    {
        std::string getfile;
        std::cout << "The 80 digit file was not found - download it? (y/n): ";
        std::cin >> getfile;

        globalTimer = std::chrono::system_clock::now();

        if (getfile == "y") {
            downloadSucceeded = download("OE_C80.txt", "http://www.aliquotes.com/OE_C80.txt", downloadFileDuration);
            if (!downloadSucceeded) {
                std::cerr << "Trouble has occurred while trying to download the 80 digit file!" << std::endl;
                return 1;
            }

            C80File.open("OE_C80.txt");

            if (!C80File.is_open()) {
                std::cerr << "Trouble has occurred while trying to read the 80 digit file!" << std::endl;
                return 1;
            }
        }
        else {
            std::cout << "Leaving..." << std::endl;
            return 0;
        }
    }
    else {
        globalTimer = std::chrono::system_clock::now();
    }

    std::string line;
    while (std::getline(C80File, line))
    {
        const auto founds = line.find(' ', 2);
        if (founds != std::string::npos) {
            const std::string matchingBase = line.substr(1, founds - 1);
            const std::string composite = line.substr(founds + 1);
            C80Map[composite] = matchingBase;
        }
    }

    std::cout << "Running base " << base << " from " << first << " through " << last << " . . ." << std::endl;

    for (unsigned int exp = first; exp <= last; exp++) {

#ifdef DEBUG
        std::cout << "Downloading base " << base << "^" << exp;
#endif

        sequenceUrl = "https://factordb.com/elf.php?seq=" + base + "^" + std::to_string(exp) + "&type=1";
        downloadSucceeded = download("aliseq1", sequenceUrl, downloadFileDuration);
        if (!downloadSucceeded) {
            std::cerr << "Trouble has occurred while trying to download the sequence " << base << "^" << exp << "!" << std::endl;
            return 1;
        }

#ifdef DEBUG
        std::cout << " : Done" << std::endl;
#endif

        bool foundC80 = false;
        ali1LastC80Composite.clear();

        ali1.open("aliseq1");
        if (ali1.is_open()) {
            while (std::getline(ali1, line)) {
                const size_t foundp = line.find('.');
                const size_t founde = line.find('=', foundp + 4);
                const std::string index = line.substr(0, foundp - 1);
                const std::string composite = line.substr(foundp + 4, founde - (1 + foundp + 4));

                ali1Map[composite] = index;

                if (composite.size() == 80) {
                    ali1LastC80Composite = composite;
                    foundC80 = true;
                }
            }
            ali1.close();
        }
        else {
            std::cerr << "aliseq1 was not read properly!" << std::endl;
            return 1;
        }

        if (!foundC80) {
            continue;
        }

        try
        {
            // Throw if not found
            const std::string matchingBase = C80Map.at(ali1LastC80Composite);
#ifdef DEBUG
            std::cout << "80 digit composite has a match in base " << matchingBase << std::endl;
#endif

#ifdef DEBUG
            std::cout << "Downloading base " << matchingBase;
#endif

            sequenceUrl = "https://factordb.com/elf.php?seq=" + matchingBase + "&type=1";
            downloadSucceeded = download("aliseq2", sequenceUrl, downloadFileDuration);
            if (!downloadSucceeded) {
                std::cerr << "Trouble has occurred while trying to download the matching sequence " << matchingBase << "!" << std::endl;
                return 1;
            }

#ifdef DEBUG
            std::cout << " : Done" << std::endl;
#endif

            ali2.open("aliseq2");
            if (ali2.is_open()) {
                while (std::getline(ali2, line))
                {
                    const size_t foundp = line.find('.');
                    const size_t founde = line.find('=');
                    const std::string index = line.substr(0, foundp - 1);
                    const std::string composite = line.substr(foundp + 4, founde - (1 + foundp + 4));

                    const auto ali1Search = ali1Map.find(composite);
                    if (ali1Search != ali1Map.end())
                    {
                        std::cout << base << "^" << exp << ":i" << ali1Search->second << " merges with " << matchingBase << ":i" << index << std::endl;
                        break;
                    }
                }
                ali2.close();
            }
            else {
                std::cerr << "aliseq2 was not read properly!" << std::endl;
                return 1;
            }
        }
        catch (const std::exception&)
        {
#ifdef DEBUG
            std::cout << "Could not find 80 digit composite " << ali1LastC80Composite << " in OE_C80.txt" << std::endl;
#endif
        }
    }

    const std::chrono::system_clock::time_point endTimer = std::chrono::system_clock::now();
    const std::chrono::system_clock::duration totalDuration = endTimer - globalTimer;
    const std::chrono::system_clock::duration computationDuration = totalDuration - downloadFileDuration;

    const auto totalMs = std::chrono::duration_cast<std::chrono::milliseconds> (totalDuration);
    const auto totalSec = std::chrono::duration_cast<std::chrono::seconds> (totalDuration);
    const auto downloadMs = std::chrono::duration_cast<std::chrono::milliseconds>(downloadFileDuration);
    const auto computeMs = std::chrono::duration_cast<std::chrono::milliseconds> (computationDuration);

    std::cout << std::endl;

    std::cout << "Total running time   : " << format_duration(totalMs) << " (" << totalSec.count() << " seconds.)" << std::endl;
    std::cout << "Downloading file time: " << format_duration(downloadMs) << std::endl;
    std::cout << "Computation only time: " << format_duration(computeMs) << std::endl;

    return 0;
}
