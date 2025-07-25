
#ifndef LOG_H
#define LOG_H
#include <fstream>
#include <mutex>
#include <string>
#include <sstream>
#include <memory>
#include <chrono>
#include <iomanip>
#include <filesystem>

namespace xlog {
	// 用 enum class（强类型枚举）避免与其他变量名冲突，是 C++11 后的推荐写法
	enum class LogLevel{		
		DEBUG=0,	// 调试信息
		INFO,		// 普通信息
		WARN,		// 警告
		ERROR,		// 错误
		FATAL		// 致命错误
	};


	// 日志管理器，负责日志的初始化、写入、轮转等
	class Logger
	{
	public:
		// 外部接口：获取唯一实例
		static Logger& getInstance();

		/*初始化：配置日志的存储路径、基础文件名、文件轮转阈值（默认 10MB）
		 参数说明：
			dir：日志文件存放目录（如 . / logs）。
			basename：日志文件基础名（如 app，最终生成 app.0.log、app.1.log 等）。
			roll_size：单个日志文件的最大大小，超过后自动创建新文件（轮转）。*/
		void init(const std::string& dir, const std::string& baseName, size_t roll_size = 10 * 1024 * 1024);
		
		void log(LogLevel lv, const std::string& file,
			int line, const std::string& msg);
		// 供宏使用
		/* 日志写入：接收 LogStream 拼接好的日志内容，处理后写入文件
		* 处理流程（内部隐含）：
			1、检查日志级别：如果 lv 低于全局设置的 level_（如全局设为 WARN，DEBUG 日志会被过滤），则不处理。
			2、加锁（std::mutex mtx_）：确保多线程环境下日志写入不冲突（线程安全）。
			3、拼接完整日志行：时间戳（now_time_str()） + 级别字符串（level_to_str(lv)） + 文件名 + 行号（file:line） + 日志内容（stream.str()）。
			4、写入文件：通过 std::ofstream ofs_ 写入当前日志文件。
			5、检查轮转：如果当前文件大小（written_）超过 roll_size_，调用 roll_file() 创建新文件。
		*/
		void log_stream(LogLevel level, const char* file, int line, std::ostringstream&& stream);

		// 设置全局级别
		void set_level(LogLevel lv) { level_ = lv; }

	private:
		// 单例
		Logger() = default;
		/* 日志轮转：当单个日志文件达到 roll_size 时，自动切换到新文件，避免文件过大难以处理
		*	1、关闭当前文件流（ofs_.close()）。
			2、生成新文件名：基础名 + 索引 + 后缀（如 app.0.log → app.1.log，通过 file_index_ 自增实现）。
			3、打开新文件（ofs_.open(new_filename)），重置 written_（当前写入大小）为 0。
		*/
		void roll_file();

		std::string level_to_str(LogLevel lv);
		std::string now_time_str();

		std::mutex mtx_;		// 	互斥锁，保证多线程写入日志的线程安全
		std::ofstream ofs_;		// 文件输出流，用于写入日志文件
		std::string baseName_;	// 日志文件基础名（初始化时传入）
		std::string dir_;		// 日志文件存放目录（初始化时传入）
		size_t roll_size_{ 0 };	// 单个日志文件最大大小（轮转阈值）
		size_t written_{ 0 };	// 当前日志文件已写入的字节数
		int file_index_{ 0 };	// 	日志文件索引（用于轮转时生成新文件名）
		LogLevel level_{ LogLevel::DEBUG };	// 全局日志级别（过滤低于此级别的日志）
	};



	/*  RAII 机制的作用
	用户创建 LogStream 对象时，传入日志级别、文件名、行号。
	通过 operator<< 向 stream_ 中拼接日志内容（如 LOG_DEBUG << "x=" << x << ", y=" << y）。
	当 LogStream 对象销毁时（通常是在语句结束时，如 ; 处），析构函数自动调用 Logger::log_stream，将拼接好的内容提交给 Logger 处理。
	优势：用户无需手动调用 “写入” 函数，语法和 cout 一样自然，且确保日志一定会被提交（即使中途发生异常，析构函数也会执行）。*/
	class LogStream {
	public:
		// 构造：记录日志级别、文件名、行号
		LogStream(LogLevel level, const char* file, int line):level_(level),file_(file),line_(line) {}
		// 析构：自动提交日志（调用Logger::log_stream）
		~LogStream() {
			Logger::getInstance().log_stream(
				level_, file_, line_, std::move(stream_));
		}

		// 重载<<：支持拼接任意类型（int、string等）
		template <typename T>
		LogStream& operator << (const T& value) { stream_ << value; return *this; }


	private:
		LogLevel level_;		// 日志级别
		const char* file_;		// 日志所在文件名（__FILE__）
		int line_;				// 日志所在行号（__LINE__）
		std::ostringstream stream_;	//  用于拼接日志内容的字符串流
	};

}

//  便携宏：隐藏 LogStream 的创建细节，自动传入当前文件名（__FILE__）和行号（__LINE__）
#define LOG_DEBUG xlog::LogStream(xlog::LogLevel::DEBUG,__FILE__,__LINE__)
#define LOG_INFO  xlog::LogStream(xlog::LogLevel::INFO,__FILE__,__LINE__)
#define LOG_WARN  xlog::LogStream(xlog::LogLevel::WARN,__FILE__,__LINE__)
#define LOG_ERROR xlog::LogStream(xlog::LogLevel::ERROR,__FILE__,__LINE__)
#define LOG_FATAL xlog::LogStream(xlog::LogLevel::FATAL,__FILE__,__LINE__)

#endif


