#include <iostream>
#include <pqxx/pqxx> 

#include "ini_parser.hpp"
#include "data_base.hpp"
#include "server.hpp"

const std::string ini_file_name = "D:\\My_Programs\\C++\\netology\\homework9.2\\diploma_thesis_2\\search_server.ini";
const std::string search_not_ready_str = "Search server is not ready to work.";


int main()
{
	setlocale(LC_ALL, "rus");
	ini_parser parser;
	parser.fill_parser(ini_file_name);

	std::string address_str;
	std::string port_str;
	std::string db_connection_string;
	int search_results;

	try
	{
		address_str = parser.get_value<std::string>("Server.host");
		port_str = parser.get_value<std::string>("Server.port");

		db_connection_string = "host=" + parser.get_value<std::string>("Database.host") + " ";	
		db_connection_string += "port=" + parser.get_value<std::string>("Database.port") + " ";	
		db_connection_string += "dbname=" + parser.get_value<std::string>("Database.dbname") + " ";	
		db_connection_string += "user=" + parser.get_value<std::string>("Database.user") + " ";	
		db_connection_string += "password=" + parser.get_value<std::string>("Database.password") + " ";

		search_results = parser.get_value<int>("Search_settings.search_results");

	}
	catch (const ParserException_no_file& ex)
	{
		std::cout << ex.what() << "\n";
		std::cout << search_not_ready_str << "\n";
		return 0;
	}
	catch (const ParserException_no_section& ex)
	{
		std::cout << ex.what() << "\n";
		std::cout << search_not_ready_str << "\n";
		return 0;
	}
	catch (const ParserException_no_variable& ex)
	{
		std::cout << ex.what() << "\n";
		std::cout << search_not_ready_str << "\n";
		return 0;
	}

	Data_base data_base(db_connection_string);
	if (!data_base.start_db())
	{
		std::cout << "Database not started. Further work is impossible. \n";

		data_base.print_last_error(); //вывести информацию о последней ошибке		
		//return 0;
	}

	auto const address = net::ip::make_address(address_str);
	auto const port = static_cast<unsigned short>(std::stoi(port_str));

	auto const doc_root = std::make_shared<std::string>("../../../..");
	auto const threads = 1;

	// The io_context is required for all I/O
	net::io_context ioc{ threads };

	// Create and launch a listening port
	std::make_shared<listener>(
		ioc,
		tcp::endpoint{ address, port },
		doc_root,
		search_results,
		&data_base)->run();

	// Run the I/O service on the requested number of threads
	std::vector<std::thread> v;
	v.reserve(threads - 1);
	for (auto i = threads - 1; i > 0; --i)
		v.emplace_back(
			[&ioc]
			{
				ioc.run();
			});
	ioc.run();
	return EXIT_SUCCESS;
}
