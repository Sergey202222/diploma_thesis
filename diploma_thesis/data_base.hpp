#pragma once
#include <iostream>
#include <pqxx/pqxx>

class Data_base
{
private:
	const std::string connection_str; 
	pqxx::connection* connection_ptr = NULL;
	std::string last_error; 
	bool connect_db(); 
	bool create_tables(); 
	bool create_templates(); 

	bool add_new_str(const std::string& str, std::string tmpl);
	int get_str_id(const std::string& str, std::string tmpl);
	bool new_word_url_pair(int url_id, int word_id, int num, std::string tmpl);

public:

	explicit Data_base(const std::string params_str)  noexcept;

	bool start_db(); 
	std::string get_last_error_desc(); 
	void print_last_error(); 

	bool test_insert(); 

	Data_base(const Data_base& other) = delete; 
	Data_base(Data_base&& other) noexcept;	
	Data_base& operator=(const Data_base& other) = delete;   
	Data_base& operator=(Data_base&& other) noexcept;      
	~Data_base();

	bool add_new_url(const std::string& url_str); 
	bool add_new_word(const std::string& word_str); 
	bool add_new_word_url_pair(int url_id, int word_id, int num); 
	bool update_word_url_pair(int url_id, int word_id, int num);

	int get_url_id(const std::string& url_str);
	int get_word_id(const std::string& word_str); 
	bool get_word_url_exist(int url_id, int word_id); 


};