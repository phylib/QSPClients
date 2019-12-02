//
// Created by phmoll on 11/25/19.
//

#include "CSVReader.h"

std::vector<std::vector<std::string>> CSVReader::getData()
{
    std::ifstream file(fileName);

    std::vector<std::vector<std::string>> dataList;

    std::string line;
    getline(file, line); // skip first row

    // Iterate through each line and split the content using delimeter
    while (getline(file, line)) {
        std::vector<std::string> vec;
        boost::algorithm::split(vec, line, boost::is_any_of(delimiter));
        dataList.push_back(vec);
    }
    // Close the File
    file.close();

    return dataList;
}