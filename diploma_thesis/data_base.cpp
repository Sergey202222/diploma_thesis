#include <map>
#include <algorithm>
#include <tuple>

//#include "Windows.h"

#include "data_base.hpp"


Data_base::Data_base(const std::string params_str) noexcept : connection_str{ params_str } {}

// конструктор перемещени€
Data_base::Data_base(Data_base&& other) noexcept : connection_str{ other.connection_str }
{
	connection_ptr = other.connection_ptr; //объект подключени€
	last_error = other.last_error;  //описание последней возникшей ошибки

	other.connection_ptr = nullptr;
}

Data_base& Data_base::operator=(Data_base&& other) noexcept   // оператор перемещающего присваивани€
{
	connection_ptr = other.connection_ptr; //объект подключени€
	last_error = other.last_error;  //описание последней возникшей ошибки

	other.connection_ptr = nullptr;
	return *this;
}

Data_base::~Data_base()
{
	delete connection_ptr;
}

bool Data_base::connect_db() //выполнить подключение к Ѕƒ
{
	try //при проблеме с подключением выбрасывает исключение
	{
		connection_ptr = new pqxx::connection(connection_str);
		return true;
	}
	catch (const pqxx::sql_error& e)
	{
		//в этот блок не попадаем, все исключени€ ловит std::exception  
		last_error = e.what();
		return false;
	}
	catch (const std::exception& ex)
	{
		last_error = ex.what();
		return false;
	}
}

std::string Data_base::get_last_error_desc() //получить описание последней возникшей ошибки
{
	return last_error;
}

void Data_base::print_last_error() //вывести информацию о последней ошибке
{
	std::cout << "Last error: " << last_error << "\n";
}

bool Data_base::create_tables() //создать необходимые таблицы
{
	if ((connection_ptr == nullptr) || (!(connection_ptr->is_open())))
	{
		last_error = "Create table error. No database connection.";
		return false;
	}

	try
	{
		pqxx::work tx{ *connection_ptr };

		//таблица urls
		tx.exec(
			"CREATE table IF NOT EXISTS documents ( "
			"id serial primary KEY, "
			"url varchar(250) NOT NULL  UNIQUE, "
			"constraint url_unique unique(url)); ");

		//таблица слов
		tx.exec(
			"CREATE table IF NOT EXISTS words ( "
			"id serial primary KEY, "
			"word varchar(32) NOT NULL  UNIQUE, "
			"constraint word_unique unique(word)); ");

		//таблица mid		
		tx.exec(
			"CREATE table IF NOT EXISTS urls_words ( "
			"word_id integer references words(id), "
			"url_id integer references documents(id), "
			"quantity integer NOT NULL,"
			"constraint pk primary key(word_id, url_id)); ");

		tx.commit();
		return true;
	}
	catch (...)
	{
		last_error = "Error creating database tables";
		return false;
	}
}

bool Data_base::create_templates() //создать шаблоны дл€ работы
{
	if ((connection_ptr == nullptr) || (!(connection_ptr->is_open())))
	{
		last_error = "Error creating query templates - no connection to database.";
		return false;
	}

	try
	{
		//добавление url
		connection_ptr->prepare("insert_url", "insert into documents (url) values ($1)");

		//добавление слова
		connection_ptr->prepare("insert_word", "insert into words (word) values ($1)");

		//получить id url страницы
		connection_ptr->prepare("search_url_id", "select id from documents where url = $1");

		//получить id слова
		connection_ptr->prepare("search_word_id", "select id from words where word = $1");

		//получить num - количество слов с word_id на странице с url_id
		connection_ptr->prepare("search_url_word_num", "select quantity from urls_words where url_id = $1 and word_id = $2");

		//добавить новое значение - количество слов  word_id на странице с url_id
		connection_ptr->prepare("add_url_word_num", "insert into urls_words(url_id, word_id, quantity) values($1, $2, $3)");

		//изменить значение - количество слов  word_id на странице с url_id		
		connection_ptr->prepare("update_url_word_num", "update urls_words set quantity = $3 where  url_id = $1 and word_id = $2");

		return true;
	}

	catch (...)
	{
		last_error = "Error creating query templates";
		return false;
	}
}

//начало работы с базой данных
bool Data_base::start_db()
{
	bool result = false;

	if (connect_db()) //подключитьс€ к базе
	{
		result = create_tables() and //создать необходимые таблицы
			create_templates(); //создать шаблоны дл€ работы; 
	}

	return result;
}

bool Data_base::test_insert() //убрать после отладки
{
	if (connection_ptr == nullptr)
	{
		last_error = "No database connection";
		return false;
	}

	try
	{
		if (!(connection_ptr->is_open()))
		{
			return false;
		}

		pqxx::work tx{ *connection_ptr };

		//добавление в таблицу пользователей
		tx.exec(
			"insert into documents (url) values "
			"('http://google.com/'), "
			"('http://google2.com/'), "
			"('http://google3.com/'); ");

		tx.commit();
	}
	catch (const std::exception& ex)
	{
		last_error = ex.what();
		return false;
	}
	catch (...)
	{
		last_error = "Error adding data";
		return false;
	}

	return true;
}

/*взаимодействие с базой данных*/
bool Data_base::add_new_str(const std::string& str, std::string tmpl)
{
	if (connection_ptr == nullptr)
	{
		last_error = "No database connection";
		return false;
	}

	last_error = "";
	try
	{
		pqxx::work tx{ *connection_ptr };
		tx.exec_prepared(tmpl, str);
		tx.commit();

		return true;
	}
	catch (const std::exception& ex)
	{
		last_error = ex.what();
		return false;
	}
}

bool Data_base::add_new_url(const std::string& url_str) //добавить новый урл
{
	return add_new_str(url_str, "insert_url");
}

bool Data_base::add_new_word(const std::string& word_str) //добавить новое слово
{
	return add_new_str(word_str, "insert_word");
}


int Data_base::get_url_id(const std::string& url_str) //узнать id url
{
	return get_str_id(url_str, "search_url_id");
}

int Data_base::get_word_id(const std::string& word_str) //узнать id слова
{
	return get_str_id(word_str, "search_word_id");
}

int Data_base::get_str_id(const std::string& str, std::string tmpl)
{
	if (connection_ptr == nullptr)
	{
		last_error = "No database connection";
		return -1;
	}

	last_error = "";

	try
	{
		pqxx::work tx{ *connection_ptr };

		auto query_res = tx.exec_prepared(tmpl, str);
		if (query_res.empty())
		{
			return -1;
		}

		auto row = query_res.begin();
		int res_int = row["id"].as<int>();
		return res_int; //вернуть первый (он же должен быть и единственным) результат
	}
	catch (const std::exception& ex)
	{
		last_error = ex.what();
		return -1;
	}
}

bool  Data_base::get_word_url_exist(int url_id, int word_id) //существует ли запись с такими id страницы и слова
{
	if (connection_ptr == nullptr)
	{
		last_error = "No database connection";
		return false;
	}

	last_error = "";
	pqxx::work tx{ *connection_ptr };

	auto query_res = tx.exec_prepared("search_url_word_num", url_id, word_id);

	if (query_res.empty()) //пустой результат		
	{
		return false;
	}
	else
		return true;
}

bool Data_base::new_word_url_pair(int url_id, int word_id, int num, std::string tmpl)
{
	if (connection_ptr == nullptr)
	{
		last_error = "No database connection";
		return false;
	}

	last_error = "";

	try
	{
		pqxx::work tx{ *connection_ptr };
		tx.exec_prepared(tmpl, url_id, word_id, num);
		tx.commit();

		return true;
	}
	catch (const std::exception& ex)
	{
		last_error = ex.what();
		return false;
	}
}

bool Data_base::add_new_word_url_pair(int url_id, int word_id, int num) //добавить новое значение - количество слов на странице
{
	return new_word_url_pair(url_id, word_id, num, "add_url_word_num");
}

bool Data_base::update_word_url_pair(int url_id, int word_id, int num) //изменить количество слов на странице
{
	return new_word_url_pair(url_id, word_id, num, "update_url_word_num");
}