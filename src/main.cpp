/**
 * @file  main.cpp
 * @brief Performance of in memory database using SQLite++ wrapper
 *
 *  
 * Copyright (c) 2012-2016 Sebastien Rombauts (sebastien.rombauts@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <ctime>
#include <random>
#include <vector>

#include <SQLiteCpp/SQLiteCpp.h>

namespace {
    const static unsigned int numberRows(100000);
    const static unsigned int numberColumns(68);
    static std::vector<std::string> values;

    struct ElapsedTime {
        ElapsedTime(const std::string &taskDescription) : _taskDescription(taskDescription), _begin(clock()) {
            std::cout << _taskDescription << " - BEGIN" << std::endl;
        }
        ~ElapsedTime() {
            std::clock_t end = clock();
            const double elapsedSecs = double(end - _begin) / CLOCKS_PER_SEC;
            std::cout << _taskDescription << " - END ";
            std::cout << "Elapsed seconds: " << elapsedSecs << std::endl;
        }
        std::string _taskDescription;
        std::clock_t _begin;
    };

}

#ifdef SQLITECPP_ENABLE_ASSERT_HANDLER
namespace SQLite
{
/// definition of the assertion handler enabled when SQLITECPP_ENABLE_ASSERT_HANDLER is defined in the project (CMakeList.txt)
void assertion_failed(const char* apFile, const long apLine, const char* apFunc, const char* apExpr, const char* apMsg)
{
    // Print a message to the standard error output stream, and abort the program.
    std::cerr << apFile << ":" << apLine << ":" << " error: assertion failed (" << apExpr << ") in " << apFunc << "() with message \"" << apMsg << "\"\n";
    std::abort();
}
}
#endif

std::string GetRandomValue() {
    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0,values.size() - 1);
    const int index = dist(rng);
    return values[index];
}

std::string CreateTableQuery() {
    std::string query = "CREATE TABLE test (id INTEGER PRIMARY KEY, ";
    for (unsigned int i = 0; i < numberColumns; i++) {
        query += "value_" + std::to_string(i) + " TEXT";
        const bool isLastColumn = (i == numberColumns - 1);
        if (not isLastColumn) query += ", ";
    }
    return query + ")";
}

std::string CreateInsertQuery() {
    std::string query = "INSERT INTO test VALUES (NULL, ";
    for (unsigned int i = 0; i < numberColumns; i++) {
        query += "\"" + GetRandomValue() + "\"";
        const bool isLastColumn = (i == numberColumns - 1);
        if (not isLastColumn) query += ", ";
    }
    return query + ")";
}

std::string CreateSelectQuery() {
    std::string query = "SELECT * FROM test WHERE ";
    for (unsigned int i = 0; i < numberColumns; i++) {
        query += "value_" + std::to_string(i) + "=\"" + GetRandomValue() + "\"";
        const bool isLastColumn = (i == numberColumns - 1);
        if (not isLastColumn) query += " AND ";
    }
    return query;
}

int main () {
    std::cout << "SQlite3 version " << SQLite::VERSION << " (" << SQLite::getLibVersion() << ")" << std::endl;
    std::cout << "SQliteC++ version " << SQLITECPP_VERSION << std::endl;

    values.push_back("val1");

    // Just one value so at this point the select is deterministic
    const std::string fixedInsertQuery = CreateInsertQuery();
    const std::string fixedSelectQuery = CreateSelectQuery();

    values.push_back("val2");
    values.push_back("val3");

    ////////////////////////////////////////////////////////////////////////////
    try
    {
        // Performance test for in memory SQLite database
        //
        SQLite::Database db(":memory:", SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
	    db.exec(CreateTableQuery());

        {
            const std::string insertQuery = CreateInsertQuery();
            ElapsedTime t("Insertion");
	        for (unsigned int i = 0; i < numberRows; i++)
	            db.exec(insertQuery);
            db.exec(fixedInsertQuery);
        }

        {
            ElapsedTime t("Select");
            SQLite::Statement query(db, fixedSelectQuery);
            while (query.executeStep()) {
                std::cout << "row (" << query.getColumn(0) << ", \"" << query.getColumn(1) << "\")\n";
            }
        }

	    db.exec("DROP TABLE test");
    }
    catch (std::exception& e)
    {
        std::cout << "SQLite exception: " << e.what() << std::endl;
        return EXIT_FAILURE; // unexpected error : exit the example program
    }

    std::cout << "everything ok, quitting\n";

    return EXIT_SUCCESS;
}
