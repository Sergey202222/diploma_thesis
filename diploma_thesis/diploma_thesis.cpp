#include <iostream>
#include <pqxx/pqxx> 
#include "Windows.h"
#include "ini_parser.hpp"
#include "spider.hpp"

const std::string ini_file_name = "spider.ini";
const std::string spider_not_ready_str = "Spider is not ready to work.";

int main()
{
	setlocale(LC_ALL, "rus");
	SetConsoleOutputCP(1251);

	ini_parser parser;
	parser.fill_parser(ini_file_name);

	Search_parameters spider_data;
	try
	{
		spider_data.start_url = parser.get_value<std::string>("URLs.start_url");

		spider_data.db_connection_string = "host=" + parser.get_value<std::string>("Database.host") + " ";	//"host=127.0.0.1 "
		spider_data.db_connection_string += "port=" + parser.get_value<std::string>("Database.port") + " ";	//"port=5432 "
		spider_data.db_connection_string += "dbname=" + parser.get_value<std::string>("Database.dbname") + " ";	//"dbname=db_lesson05 "
		spider_data.db_connection_string += "user=" + parser.get_value<std::string>("Database.user") + " ";	//"user=lesson05user "
		spider_data.db_connection_string += "password=" + parser.get_value<std::string>("Database.password") + " ";//"password=lesson05user");

		spider_data.search_depth = parser.get_value<int>("Search_settings.search_depth");
		spider_data.max_word_length = parser.get_value<int>("Search_settings.max_word_length");
		spider_data.min_word_length = parser.get_value<int>("Search_settings.min_word_length");

		spider_data.max_threads_num = parser.get_value<int>("Process_settings.max_threads");
		spider_data.empty_thread_sleep_time = parser.get_value<int>("Process_settings.empty_thread_sleep_time");
		spider_data.this_host_only = parser.get_value<bool>("Process_settings.this_host_only");
	}
	catch (const ParserException_no_file& ex)
	{
		std::cout << ex.what() << "\n";
		std::cout << spider_not_ready_str << "\n";
		return EXIT_SUCCESS;
	}
	catch (const ParserException_no_section& ex)
	{
		std::cout << ex.what() << "\n";
		std::cout << spider_not_ready_str << "\n";
		return EXIT_SUCCESS;
	}
	catch (const ParserException_no_variable& ex)
	{
		std::cout << ex.what() << "\n";
		std::cout << spider_not_ready_str << "\n";
		return EXIT_SUCCESS;
	}

	std::cout << "Indexing information:________ " << std::endl;
	std::cout << "Start url = " << spider_data.start_url << "\n\n";
	std::cout << "Indexing settings: " << std::endl;
	std::cout << "Search_depth = " << spider_data.search_depth << "\n";
	std::cout << "Minimum word length for indexing = " << spider_data.min_word_length << "\n";
	std::cout << "Maximum word length for indexing = " << spider_data.max_word_length << "\n\n";
	std::cout << "Database settings: " << spider_data.db_connection_string << std::endl;

	Spider spider(spider_data);
	std::cout << "Start indexing from " << spider_data.start_url << std::endl;
	spider.start_spider(); //старт пула потоков паука			



	return EXIT_SUCCESS;
}
