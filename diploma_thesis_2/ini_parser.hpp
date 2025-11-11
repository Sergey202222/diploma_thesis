#pragma once
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <tuple>
#include <regex>

#include "parser_exceptions.hpp"

enum class string_type
{
	section_,
	variable_,
	empty_,
	invalid_
};

class ini_parser
{
private:
	bool parser_invalid = false;
	bool file_read = false;
	bool invalid_data = false;

	int incorrect_str_num = 0;
	std::string incorrect_str;

private:
	std::vector<std::map<std::string, std::string>>* variables_str_array = nullptr;
	std::map<std::string, int>* sections_map = nullptr;

	std::string current_section_name;
	int current_section_number = -1;

	std::string delete_spaces(const std::string& src_str);
	std::tuple<string_type, std::string, std::string> research_string(const std::string& src_str); 
	std::string get_section_name(const int section_index);
	int get_section_number(const std::string& section_name);
	std::tuple<std::string, std::string> get_section_variable_names(const std::string& src_str);
	std::string get_variable_value(const int section_index, const std::string& variable_name);

public:
	ini_parser();
	ini_parser(const ini_parser& other);
	ini_parser& operator = (const ini_parser& other);
	ini_parser(ini_parser&& other) noexcept;
	ini_parser& operator=(ini_parser&& other) noexcept;
	~ini_parser();

	void fill_parser(const std::string& file_name);
	bool print_all_sections();
	bool print_all_sections_info();
	bool print_all_variables(const std::string& section_name);
	void check_parser();
	void print_incorrect_info();
	std::string get_section_from_request(const std::string& request_str);

	template <class T>
	T get_value(const std::string& input_str)
	{
		try
		{
			check_parser(); 
		}
		catch (const ParserException_incorrect_data& ex)
		{
			std::cout << ex.what() << "\n";
		}

		std::string section_name;
		std::string variable_name;

		std::tie(section_name, variable_name) = get_section_variable_names(input_str);

		int section_index = get_section_number(section_name);
		if (section_index < 0)
		{
			throw ParserException_no_section();
		}

		std::string variable_value = get_variable_value(section_index, variable_name);

		variable_value = std::regex_replace(variable_value, std::regex("([ \t])"), "");

		if constexpr (std::is_same_v<T, int>)
		{
			try
			{
				return std::stoi(variable_value);
			}
			catch (...)
			{
				throw ParserException_no_variable();
			}
		}
		else if constexpr (std::is_same_v<T, double>)
		{
			try
			{
				return std::stod(variable_value);
			}
			catch (...)
			{
				throw ParserException_no_variable();
			}
		}
		else if constexpr (std::is_same_v<T, std::string>)
		{
			return variable_value;
		}
		else if constexpr (std::is_same_v<T, bool>)
		{

			return (variable_value != "0");
		}
		else
		{
			throw ParserException_type_error();
		};

	}

};