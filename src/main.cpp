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
    std::string xlsx_file;
    bool no_pretty = false;
    bool has_arg_error = false;
    bool skip_empty_row = false;
    auto tz = dateutil::local_tz_seconds();

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg[0] == '-') {
            if (arg == "--no-pretty") {
                no_pretty = true;
            } else if (arg == "--skip-empty-row") {
                skip_empty_row = true;
            } else {
                has_arg_error = true;
            }
        } else {
            xlsx_file = arg;
        }
    }
    if (has_arg_error || xlsx_file.empty()) {
        std::cerr <<
            "usage: xlsx2lua " <<
            "[--no-pretty] " <<
            "[--skip-empty-row] " <<
            "[--timezone '+00:00'] " <<
            "xlsx_file" <<
            std::endl;
        return 1;
    }

    auto sp = no_pretty ? "" : " ";
    auto nl = no_pretty ? "" : "\n";
    auto idt = no_pretty ? "" : "  ";

    xlsx::Workbook book(xlsx_file);
    std::cout << "return" << sp << "{" << nl;
    std::cout << idt << "sheets" << sp << "=" << sp << "{" << nl;
    for (int i = 0; i < book.nsheets(); ++i) {
        auto& sheet = book.sheet(i);
        if (i != 0) {
            std::cout << "," << nl;
        }
        std::cout <<
            idt << idt << "{" << sp << "name" << sp << "=" << sp <<
            lua_escape(sheet.name) << "," << nl;
        std::cout <<
            idt << idt << idt << "cells" << sp << "=" << sp << "{";
        auto firstrow = true;
        for (int y = 0; y < sheet.nrows(); ++y) {
            int ncols = -1;
            for (int x = sheet.ncols() - 1; x >= 0; --x) {
                if (!sheet.cell(y, x).empty()) {
                    ncols = x + 1;
                    break;
                }
            }
            if (skip_empty_row && ncols == -1) {
                continue;
            }
            std::cout << nl;
            std::cout << idt << idt << idt << idt << "{";
            firstrow = false;
            for (int x = 0; x < ncols; ++x) {
                if (x > 0) {
                    std::cout << ",";
                }
                auto& cell = sheet.cell(y, x);
                switch (cell.type) {
                    // kEmpty, kString, kInt, kDouble, kDateTime, kBool
                    case xlsx::Cell::Type::kEmpty:
                        std::cout << "\"\"";
                        break;
                    case xlsx::Cell::Type::kString:
                        std::cout << lua_escape(cell.as_str());
                        break;
                    case xlsx::Cell::Type::kInt:
                        std::cout << cell.as_int();
                        break;
                    case xlsx::Cell::Type::kDouble:
                        std::cout << cell.as_double();
                        break;
                    case xlsx::Cell::Type::kDateTime:
                        std::cout << lua_escape(dateutil::isoformat64(cell.as_time64(tz), tz));
                        break;
                    case xlsx::Cell::Type::kBool:
                        std::cout << (cell.as_bool() ? "true" : "false");
                        break;
                }
            }
            if (y == sheet.nrows() - 1) {
                std::cout << "}";
            } else {
                std::cout << "},";
            }
        }
        std::cout << nl << idt << idt << idt << "}";
        std::cout << nl << idt << idt << "}";
    }
    std::cout << nl << idt << "}";
    std::cout << nl << "}\n";

    return 0;
}

