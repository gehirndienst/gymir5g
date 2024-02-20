/*
 * CSVLogger.h
 *
 * c++ csv logger class
 *
 * Created on: Feb 17, 2023
 *      Author: Nikita Smirnov
 */

#include <chrono>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#ifndef CSVLOGGER_H
#define CSVLOGGER_H

class CSVLogger {
public:
    CSVLogger(const std::string& filepath, const std::string& separator = ",")
        : fstream_(), separator_(separator), isFirstChar(true) {
        if (isFilename(filepath)) {
            openFile(filepath);
        } else {
            createFile(filepath);
        }
    }

    ~CSVLogger() {
        fstream_.flush();
        fstream_.close();
    }

    void endl() {
        fstream_ << std::endl;
        isFirstChar = true;
    }
    // use with "<<" operator same as std::endl
    static CSVLogger& endl(CSVLogger& self) {
        self.endl();
        return self;
    }

    static inline std::string getCurrentTimestamp() {
        // add current timestamp with mss to the filename
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        auto timer = std::chrono::system_clock::to_time_t(now);
        std::tm bt = *std::localtime(&timer);
        std::ostringstream oss;
        oss << std::put_time(&bt, "%Y_%m_%d-%H_%M_%S");
        oss << '_' << std::setfill('0') << std::setw(3) << ms.count();
        return oss.str();
    }

    bool getIsHeaderWritten() const { return isHeaderWritten; }

public:
    // ostream operator overloading, use only this operator for writing!
    CSVLogger& operator << (CSVLogger & (*self)(CSVLogger&)) { return self(*this); }
    CSVLogger& operator << (const char* cstr) { return write(escape(std::string(cstr))); }
    CSVLogger& operator << (const std::string& stdstr) { return write(escape(stdstr)); }
    template<typename T> CSVLogger& operator << (const T& num) { return write(num); }

private:
    // main writing method
    template<typename T> CSVLogger& write(const T& val) {
        if (!isFirstChar) {
            fstream_ << separator_;
        } else {
            isFirstChar = false;
        }
        if (!isHeaderWritten) {
            isHeaderWritten = true;
        }
        fstream_ << val;
        return *this;
    }

    // prevent trailing separator and escape logic
    std::string escape(const std::string& input) {
        std::ostringstream res;
        res << '"';
        std::string::size_type to, from = 0u, len = input.length();
        while (from < len && std::string::npos != (to = input.find_first_of("\"", from))) {
            res << input.substr(from, to - from) << "\"" << input[to];
            from = to + 1;
        }
        res << input.substr(from) << '"';
        return res.str();
    }

    // check if the given path is a filename with a .csv extension
    bool isFilename(const std::string& path) {
        std::filesystem::path fsPath(path);
        return fsPath.has_filename() && fsPath.extension() == ".csv";
    }

    // append to an existing file
    void openFile(const std::string& filename) {
        fstream_.exceptions(std::ios::failbit | std::ios::badbit);
        fstream_.open(filename, std::ofstream::out | std::ofstream::app);
        isHeaderWritten = !std::filesystem::is_empty(filename);
    }

    // create a new file with a timestamp
    void createFile(const std::string& path) {
        std::string filepathToWrite = path + getCurrentTimestamp() + ".csv";
        fstream_.exceptions(std::ios::failbit | std::ios::badbit);
        fstream_.open(filepathToWrite, std::ofstream::out | std::ofstream::app);
        isHeaderWritten = false;
    }

private:
    std::ofstream fstream_;
    const std::string separator_;
    bool isFirstChar;
    bool isHeaderWritten;
};

#endif