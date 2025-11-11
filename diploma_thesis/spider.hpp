#pragma once
#include <iostream>
#include <vector>
#include <thread>
#include <string>
#include <set>
#include <map>

#include "data_base.hpp"
#include "tasks_queue.hpp"
#include "html_parser.hpp"

struct Search_parameters
{
	std::string start_url;
	int	search_depth = 1;
	int	min_word_length = 3;
	int	max_word_length = 32;
	int max_threads_num = 0;
	int empty_thread_sleep_time = 100;
	bool this_host_only = false;
	std::string db_connection_string;
};



//class Spider;

class url_processing_thread : public std::thread
{
public:
	bool in_work = false;
	url_item thread_task;
};

class Spider //thread_pool
{
private:
	enum  class host_type
	{
		host_http = 0,
		host_https,
		host_unknown
	};

	enum class request_result
	{
		req_res_unknown = 0,
		req_res_ok = 200,
		req_res_redirect = 301

	};

	bool spider_invalid = false; //есть проблемы с пауком
	Data_base* data_base = nullptr;
	std::string db_connection_string;

	unsigned int max_depth = 0;
	bool this_host_only = 0;
	int max_threads_num = 0;
	std::string start_url;

	int total_pages_processed = 0;
	std::vector<url_processing_thread> th_vector;

	tasks_queue  pool_queue;
	html_parser my_html_parser;

	std::atomic<bool> task_generator_finished(const int max_threads_num);
	bool start_work = false;

	std::mutex threads_start_mutex; //мьютекс старта работы потоков
	std::mutex threads_get_state; //мьютекс запроса состояния
	std::mutex data_base_mutex; //мьютекс обращения к базе данных
	std::condition_variable start_threads;

	void start_work_threads(); //создать и запустить рабочие потоки
	void submit(const url_item new_url_item, const int work_thread_num); //добавление адреса в очередь

	void work(const int& thread_index); //рабочая функция потоков
	bool process_next_task(const int& thread_index);
	bool work_function(const url_item& new_url_item, std::set<std::string>& new_urls_set, std::map<std::string, unsigned  int>& new_words_map);

	host_type get_host_type(std::string& new_url); //определить, по какому протоколу скачивать страницу и скачивать ли
	void get_host_and_target(std::string& new_url, std::string& host, std::string& target); //получить host и target
	bool url_is_forbidden(const std::string& check_host); //не входит ли хост в список запрещенных для индексирования
	void add_url_words_to_database(const std::string& url_str, const std::map<std::string, unsigned  int>& words_map);

public:

	std::vector<std::string> forbidden_urls{ "facebook.com" }; //{ "instagram.com", "facebook.com" }; //список запрещенных для индексирования хостов

	Spider(Search_parameters spider_data);
	~Spider();

	Spider(const Spider& other) = delete; 
	Spider(Spider&& other) noexcept;	
	Spider& operator=(const Spider& other) = delete;  
	Spider& operator=(Spider&& other) = delete;       

	void start_spider(); 

	std::string get_queue_state(); //получить информацию о работе - очередь и пройденные урлы
	std::string get_threads_state(); //получить информацию о рабочих потоках  - не блокирует мьютексы
	void print_urls_list();	


};