// Copyright (c) 2016 peposso All Rights Reserved.
// Released under the MIT license
#include <string>
#include <vector>
#include <utility>
#include <functional>
#include <thread>
#include <mutex>

#include "xlsx.hpp"
#include "dateutil.hpp"

std::string lua_escape(const std::string& s) {
    std::string r;
    r.push_back('"');
    for (auto c : s) {
        if (c == '\t') {
            r.push_back('\\');
            r.push_back('t');
        } else if (c == '\r') {
            r.push_back('\\');
            r.push_back('r');
        } else if (c == '\n') {
            r.push_back('\\');
            r.push_back('n');
        } else if (c == '"') {
            r.push_back('\\');
            r.push_back('"');
        } else if (c == '\\') {
            r.push_back('\\');
            r.push_back('\\');
        } else {
            r.push_back(c);
        }
    }
    r.push_back('"');
    return r;
}

int main(int argc, char** argv) {
    std::string input_file;

    if (argc != 2) {
        std::cerr << argv[0] << " {input-xlsx}" << std::endl;
        return 1;
    }
    input_file = argv[1];
    auto tz = dateutil::local_tz_seconds();
    std::cerr << "local_tz_seconds:" << tz << std::endl;

    xlsx::Workbook book(input_file);

    std::cout << "return {\n";
    std::cout << "  sheets = {\n";
    for (int i = 0; i < book.nsheets(); ++i) {
        auto& sheet = book.sheet(i);
        std::cout << "    { name = " << lua_escape(sheet.name) << ",\n";
        std::cout << "      cells = {\n";
        for (int r = 0; r < sheet.nrows(); ++r) {
            std::cout << "        {";
            int ncols = -1;
            for (int c = sheet.ncols() - 1; c >= 0; --c) {
                if (!sheet.cell(r, c).empty()) {
                    ncols = c + 1;
                    break;
                }
            }
            for (int c = 0; c < ncols; ++c) {
                auto& cell = sheet.cell(r, c);
                switch (cell.type) {
                    // kEmpty, kString, kInt, kDouble, kDateTime, kBool
                    case xlsx::Cell::Type::kEmpty:
                        std::cout << "\"\",";
                        break;
                    case xlsx::Cell::Type::kString:
                        std::cout << lua_escape(cell.as_str()) << ",";
                        break;
                    case xlsx::Cell::Type::kInt:
                        std::cout << cell.as_int() << ",";
                        break;
                    case xlsx::Cell::Type::kDouble:
                        std::cout << cell.as_double() << ",";
                        break;
                    case xlsx::Cell::Type::kDateTime:
                        std::cout << lua_escape(dateutil::isoformat64(cell.as_time64(tz), tz)) << ",";
                        break;
                    case xlsx::Cell::Type::kBool:
                        std::cout << (cell.as_bool() ? "true" : "false") << ",";
                        break;
                }
            }
            std::cout << "},\n";
        }
        std::cout << "      },\n";
        std::cout << "    },\n";
    }
    std::cout << "  },\n";
    std::cout << "}\n";

    return 0;
}

