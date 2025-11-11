#pragma once
#include <iostream>
#include <pqxx/pqxx> 
#include <map>
#include <set>

class Data_base
{
private:
	const std::string connection_str; //строка параметров дл€ подключени€
	pqxx::connection* connection_ptr = NULL;
	std::string last_error; //описание последней возникшей ошибки
	bool connect_db(); //выполнить подключение к Ѕƒ
	bool create_tables(); //создать необходимые таблицы


public:

	explicit Data_base(const std::string params_str)  noexcept;

	bool start_db(); //начало работы с базой данных
	std::string get_last_error_desc(); //получить описание последней возникшей ошибки
	void print_last_error(); //вывести информацию о последней ошибке

	Data_base(const Data_base& other) = delete; //конструктор копировани€
	Data_base(Data_base&& other) noexcept;	// конструктор перемещени€
	Data_base& operator=(const Data_base& other) = delete;  //оператор присваивани€ 
	Data_base& operator=(Data_base&& other) noexcept;       // оператор перемещающего присваивани€
	~Data_base();


	//Search*******************************************************

	std::map<std::string, int>  get_urls_list_by_words(const std::set<std::string>& words_set);//получить мап адресов, по которым встречаютс€ искомые слова
	int count_url_words(const std::set<std::string>& words_set, std::string url); //посчитать, сколько из запрашиваемых слов есть по этому адресу
	std::multimap<std::string, int> get_words_urls_table(const std::set<std::string>& words_set); //получить из базы все записи с адресами и количеством вхождений слов
	std::string prepare_words_where_or(const std::set<std::string>& words_set, pqxx::work& tx); //подготовить выражение where or из запрашиваемых слов


};