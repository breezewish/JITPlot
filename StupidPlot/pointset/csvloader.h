#pragma once

#include <stdexcept>
#include <fstream>
#include <string>
#include <vector>

namespace StupidPlot
{
    namespace Pointset
    {
        class CSVLoader
        {
        public:
            static void load(
                std::wstring fileName,
                std::vector<double> & x,
                std::vector<double> & y)
            {
                std::wifstream fin(fileName);
                if (!fin.good()) throw std::runtime_error("Failed to open file");

                std::wstring line;
                while (std::getline(fin, line))
                {
                    if (line.size() > 0)
                    {
                        auto p = line.find_first_of(L',');
                        if (p != std::wstring::npos)
                        {
                            x.push_back(std::stof(line.substr(0, p)));
                            y.push_back(std::stof(line.substr(p + 1)));
                        }
                    }
                }
            }
        };
    }
}