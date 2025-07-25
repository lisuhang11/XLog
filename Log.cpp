#define _CRT_SECURE_NO_WARNINGS
#include "Log.h"
#include <thread>
#include <iostream>

using namespace xlog;


Logger& Logger::getInstance() {
	static Logger ins;
	return ins;
}

void Logger::init(const std::string& dir, const std::string& baseName, size_t roll_size) {
	std::lock_guard<std::mutex> lock(mtx_);
	dir_ = dir;
	baseName_ = baseName;
	roll_size_ = roll_size;
	std::filesystem::create_directories(dir_);
	roll_file();
}

void Logger::roll_file() {
	if (ofs_.is_open()) ofs_.close();
	std::string path = dir_ + "/" + baseName_ + "." +
		std::to_string(file_index_++) + ".log";
	ofs_.open(path, std::ios::out | std::ios::app);
	written_ = 0;
}

std::string Logger::level_to_str(LogLevel lv) {
	switch (lv) {
	case LogLevel::DEBUG: return "DEBUG";
	case LogLevel::INFO: return "INFO ";
	case LogLevel::WARN: return "WARN ";
	case LogLevel::ERROR: return "ERROR";
	case LogLevel::FATAL: return "FATAL";
	}
	return "UNKNOWN";
}

std::string Logger::now_time_str() {
	auto now = std::chrono::system_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		now.time_since_epoch()) % 1000;
	std::time_t t = std::chrono::system_clock::to_time_t(now);
	std::tm tm = *std::localtime(&t);
	char buf[32];
	std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%03lld",
		tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		tm.tm_hour, tm.tm_min, tm.tm_sec,
		static_cast<long long>(ms.count()));
	return buf;
}

void Logger::log(LogLevel lv, const std::string& file, int line, const std::string& msg) {
	if (lv < level_) {
		return;
	}
	std::lock_guard<std::mutex> lock(mtx_);
	std::string line_str = now_time_str() + " [ " + level_to_str(lv) + " ] " + file + " : " + std::to_string(line) + " | " + msg + "\n";
	if (ofs_.is_open()) {
		ofs_.write(line_str.c_str(), line_str.size());
		written_ += line_str.size();
		if (written_ > roll_size_){
			roll_file();
		}
	}
	std::cout << line_str << std::flush;

}



void Logger::log_stream(LogLevel lv, const char* file, int line,
	std::ostringstream&& stream) {
	log(lv, file, line, stream.str());
}