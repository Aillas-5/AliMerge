// This program is designed to look for merges of Aliquot
// sequences.  It uses the file OE_3000000_C80.txt to
// match the last occurrence of an 80 digit composite and
// determines the merged sequence, if any.  If a merge is
// found, the program determines the merge points between
// the two sequences.  This program is specifically built
// to work with base tables found at:
//    "Aliquot sequences starting on integer powers n^i"
// (http://www.aliquotes.com/aliquotes_puissances_entieres.html)
//
// It can be compiled in linux via:
//           g++ <sourcename> -o <finalname>
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
#include <cstdio>

std::string format_duration(std::chrono::milliseconds ms) {
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

int main(int argc, char** argv) {

    std::ifstream ali1, ali2;
    std::string ali1LastC80Composite, commandStr;
    std::string base;
    int first, last, exp;

    std::map<std::string, std::string> C80Map;
    std::map<std::string, std::string> ali1Map;

    if (argc < 4) {
        std::cerr << "Please invoke as: <./program> <base> <starting exponent> <ending exponent>" << std::endl;
        return 1;
    }

    std::chrono::system_clock::duration downloadFileDuration = std::chrono::system_clock::duration::zero();
    std::chrono::system_clock::duration computationDuration = std::chrono::system_clock::duration::zero();
    std::chrono::system_clock::duration totalDuration = std::chrono::system_clock::duration::zero();
    std::chrono::system_clock::time_point globalTimer;
    std::chrono::system_clock::time_point startTimer;
    std::chrono::system_clock::time_point endTimer;

    base = argv[1];

#ifdef _WIN32
    sscanf_s(argv[2], "%d", &first);
    sscanf_s(argv[3], "%d", &last);
#else
    sscanf(argv[2], "%d", &first);
    sscanf(argv[3], "%d", &last);
#endif

    std::ifstream C80File("OE_5000000_C80.txt");

    if (C80File.fail())
    {
        std::string getfile;
        std::cout << "The 80 digit file was not found - download it? (y/n): ";
        std::cin >> getfile;

        globalTimer = std::chrono::system_clock::now();

        if (getfile == "y") {
            startTimer = std::chrono::system_clock::now();
            system(R"(curl -q -s -o OE_5000000_C80.txt "http://www.aliquotes.com/OE_5000000_C80.txt")");
            endTimer = std::chrono::system_clock::now();
            downloadFileDuration += endTimer - startTimer;

            C80File.open("OE_5000000_C80.txt");

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
        auto founds = line.find(' ', 2);
        if (founds != std::string::npos) {
            std::string matchingBase = line.substr(1, founds - 1);
            std::string composite = line.substr(founds + 1);
            C80Map[composite] = matchingBase;
        }
    }

    std::cout << "Running base " << base << " from " << first << " through " << last << " . . ." << std::endl;

    for (exp = first; exp <= last; exp++) {

        startTimer = std::chrono::system_clock::now();
#ifdef DEBUG
        std::cout << "Downloading base " << base << "^" << exp;
#endif

        commandStr = R"(curl -q -s -o aliseq1 "http://www.factordb.com/elf.php?seq=)" + base + "^" + std::to_string(exp) + R"(&type=1")";
        system(commandStr.c_str());

#ifdef DEBUG
        std::cout << " : Done" << std::endl;
#endif
        endTimer = std::chrono::system_clock::now();
        downloadFileDuration += endTimer - startTimer;

        bool foundC80 = false;
        ali1LastC80Composite.clear();

        ali1.open("aliseq1");
        if (ali1.is_open()) {
            while (std::getline(ali1, line)) {
                size_t foundp = line.find('.');
                size_t founde = line.find('=', foundp + 4);
                std::string index = line.substr(0, foundp - 1);
                std::string composite = line.substr(foundp + 4, founde - (1 + foundp + 4));

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
            std::string matchingBase = C80Map.at(ali1LastC80Composite);
#ifdef DEBUG
            std::cout << "80 digit composite has a matching in base " << matchingBase << std::endl;
#endif

            startTimer = std::chrono::system_clock::now();
#ifdef DEBUG
            std::cout << "Downloading base " << matchingBase;
#endif

            commandStr = R"(curl -q -s -o aliseq2 "http://www.factordb.com/elf.php?seq=)" + matchingBase + R"(&type=1")";
            system(commandStr.c_str());

#ifdef DEBUG
            std::cout << " : Done" << std::endl;
#endif
            endTimer = std::chrono::system_clock::now();
            downloadFileDuration += endTimer - startTimer;

            ali2.open("aliseq2");
            if (ali2.is_open()) {
                while (std::getline(ali2, line))
                {
                    size_t foundp = line.find('.');
                    size_t founde = line.find('=');
                    std::string index = line.substr(0, foundp - 1);
                    std::string composite = line.substr(foundp + 4, founde - (1 + foundp + 4));

                    auto ali1Search = ali1Map.find(composite);
                    if (ali1Search != ali1Map.end())
                    {
                        std::cout << base + "^" + std::to_string(exp) + ":i" + ali1Search->second + " merges with " + matchingBase + ":i" + index << std::endl;
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
            std::cout << "Could not find 80 digit composite " << ali1LastC80Composite << " in OE_3000000_C80.txt" << std::endl;
#endif
        }
    }

    endTimer = std::chrono::system_clock::now();
    totalDuration = endTimer - globalTimer;
    computationDuration = totalDuration - downloadFileDuration;

    auto totalMs = std::chrono::duration_cast<std::chrono::milliseconds> (totalDuration);
    auto totalSec = std::chrono::duration_cast<std::chrono::seconds> (totalDuration);
    auto downloadMs = std::chrono::duration_cast<std::chrono::milliseconds>(downloadFileDuration);
    auto computeMs = std::chrono::duration_cast<std::chrono::milliseconds> (computationDuration);

    std::cout << std::endl;

    std::cout << "Total running time   : " << format_duration(totalMs) << " (" << totalSec.count() << " seconds.)" << std::endl;
    std::cout << "Downloading file time: " << format_duration(downloadMs) << std::endl;
    std::cout << "Computation only time: " << format_duration(computeMs) << std::endl;

    return 0;
}
