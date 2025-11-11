#pragma once
#include <iostream>
#include <mutex>
#include <queue>
#include <functional>
#include <condition_variable>
#include <string>
#include <set>

struct url_item
{
	std::string url;
	int url_depth;
};

class tasks_queue
{
private:
	std::queue<url_item>   urls_queue;	//очередь урлов на сканирование	 
	std::mutex queue_mutex;				//мьютекс для доступа к очереди
	std::condition_variable data_cond;


public:
	unsigned int empty_sleep_for_time = 100; //таймаут сна для незанятых потоков
	std::set<std::string> list_of_urls;		//общий список всех собранных урлов

	void sq_push(const url_item& new_url_item, const int work_thread_num); //добавить урл в очередь
	bool sq_pop(url_item& task, const int work_thread_num); //забрать урл из очереди
	bool not_empty();
	bool is_empty();

	int get_queue_size(); 
};