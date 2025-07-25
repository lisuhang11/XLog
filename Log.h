
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
	// �� enum class��ǿ����ö�٣�������������������ͻ���� C++11 ����Ƽ�д��
	enum class LogLevel{		
		DEBUG=0,	// ������Ϣ
		INFO,		// ��ͨ��Ϣ
		WARN,		// ����
		ERROR,		// ����
		FATAL		// ��������
	};


	// ��־��������������־�ĳ�ʼ����д�롢��ת��
	class Logger
	{
	public:
		// �ⲿ�ӿڣ���ȡΨһʵ��
		static Logger& getInstance();

		/*��ʼ����������־�Ĵ洢·���������ļ������ļ���ת��ֵ��Ĭ�� 10MB��
		 ����˵����
			dir����־�ļ����Ŀ¼���� . / logs����
			basename����־�ļ����������� app���������� app.0.log��app.1.log �ȣ���
			roll_size��������־�ļ�������С���������Զ��������ļ�����ת����*/
		void init(const std::string& dir, const std::string& baseName, size_t roll_size = 10 * 1024 * 1024);
		
		void log(LogLevel lv, const std::string& file,
			int line, const std::string& msg);
		// ����ʹ��
		/* ��־д�룺���� LogStream ƴ�Ӻõ���־���ݣ������д���ļ�
		* �������̣��ڲ���������
			1�������־������� lv ����ȫ�����õ� level_����ȫ����Ϊ WARN��DEBUG ��־�ᱻ���ˣ����򲻴���
			2��������std::mutex mtx_����ȷ�����̻߳�������־д�벻��ͻ���̰߳�ȫ����
			3��ƴ��������־�У�ʱ�����now_time_str()�� + �����ַ�����level_to_str(lv)�� + �ļ��� + �кţ�file:line�� + ��־���ݣ�stream.str()����
			4��д���ļ���ͨ�� std::ofstream ofs_ д�뵱ǰ��־�ļ���
			5�������ת�������ǰ�ļ���С��written_������ roll_size_������ roll_file() �������ļ���
		*/
		void log_stream(LogLevel level, const char* file, int line, std::ostringstream&& stream);

		// ����ȫ�ּ���
		void set_level(LogLevel lv) { level_ = lv; }

	private:
		// ����
		Logger() = default;
		/* ��־��ת����������־�ļ��ﵽ roll_size ʱ���Զ��л������ļ��������ļ��������Դ���
		*	1���رյ�ǰ�ļ�����ofs_.close()����
			2���������ļ����������� + ���� + ��׺���� app.0.log �� app.1.log��ͨ�� file_index_ ����ʵ�֣���
			3�������ļ���ofs_.open(new_filename)�������� written_����ǰд���С��Ϊ 0��
		*/
		void roll_file();

		std::string level_to_str(LogLevel lv);
		std::string now_time_str();

		std::mutex mtx_;		// 	����������֤���߳�д����־���̰߳�ȫ
		std::ofstream ofs_;		// �ļ������������д����־�ļ�
		std::string baseName_;	// ��־�ļ�����������ʼ��ʱ���룩
		std::string dir_;		// ��־�ļ����Ŀ¼����ʼ��ʱ���룩
		size_t roll_size_{ 0 };	// ������־�ļ�����С����ת��ֵ��
		size_t written_{ 0 };	// ��ǰ��־�ļ���д����ֽ���
		int file_index_{ 0 };	// 	��־�ļ�������������תʱ�������ļ�����
		LogLevel level_{ LogLevel::DEBUG };	// ȫ����־���𣨹��˵��ڴ˼������־��
	};



	/*  RAII ���Ƶ�����
	�û����� LogStream ����ʱ��������־�����ļ������кš�
	ͨ�� operator<< �� stream_ ��ƴ����־���ݣ��� LOG_DEBUG << "x=" << x << ", y=" << y����
	�� LogStream ��������ʱ��ͨ������������ʱ���� ; ���������������Զ����� Logger::log_stream����ƴ�Ӻõ������ύ�� Logger ����
	���ƣ��û������ֶ����� ��д�롱 �������﷨�� cout һ����Ȼ����ȷ����־һ���ᱻ�ύ����ʹ��;�����쳣����������Ҳ��ִ�У���*/
	class LogStream {
	public:
		// ���죺��¼��־�����ļ������к�
		LogStream(LogLevel level, const char* file, int line):level_(level),file_(file),line_(line) {}
		// �������Զ��ύ��־������Logger::log_stream��
		~LogStream() {
			Logger::getInstance().log_stream(
				level_, file_, line_, std::move(stream_));
		}

		// ����<<��֧��ƴ���������ͣ�int��string�ȣ�
		template <typename T>
		LogStream& operator << (const T& value) { stream_ << value; return *this; }


	private:
		LogLevel level_;		// ��־����
		const char* file_;		// ��־�����ļ�����__FILE__��
		int line_;				// ��־�����кţ�__LINE__��
		std::ostringstream stream_;	//  ����ƴ����־���ݵ��ַ�����
	};

}

//  ��Я�꣺���� LogStream �Ĵ���ϸ�ڣ��Զ����뵱ǰ�ļ�����__FILE__�����кţ�__LINE__��
#define LOG_DEBUG xlog::LogStream(xlog::LogLevel::DEBUG,__FILE__,__LINE__)
#define LOG_INFO  xlog::LogStream(xlog::LogLevel::INFO,__FILE__,__LINE__)
#define LOG_WARN  xlog::LogStream(xlog::LogLevel::WARN,__FILE__,__LINE__)
#define LOG_ERROR xlog::LogStream(xlog::LogLevel::ERROR,__FILE__,__LINE__)
#define LOG_FATAL xlog::LogStream(xlog::LogLevel::FATAL,__FILE__,__LINE__)

#endif


