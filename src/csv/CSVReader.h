//
// Created by phmoll on 11/25/19.
//

#ifndef QUADTREESYNCEVALUATION_CSVREADER_H
#define QUADTREESYNCEVALUATION_CSVREADER_H

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

/*
 * A class to read data from a csv file.
 *
 * From: https://thispointer.com/how-to-read-data-from-a-csv-file-in-c/
 */
class CSVReader {
    std::string fileName;
    std::string delimiter;

public:
    explicit CSVReader(const std::string& filename, const std::string& delm = "\t")
        : fileName(filename)
        , delimiter(delm)
    {
    }

    /*
     * Parses through csv file line by line and returns the data
     * in vector of vector of strings.
     */
    std::vector<std::vector<std::string>> getData();
};

#endif // QUADTREESYNCEVALUATION_CSVREADER_H
