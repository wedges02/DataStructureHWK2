#include <iostream>
#include "csv_io.h"

int main()
{
    std::map<int, std::vector<Point>> val = ParseInputCSV("test_cases/input_original_01.csv");

    for(auto& [val1,val2]: val)
    {
        for(auto val3 : val2)
            std::cout << val1 << " ,Points: " << val3.x << ", " << val3.y <<std::endl;
    }

    return 0;
}