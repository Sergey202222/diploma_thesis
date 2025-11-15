#include "ini_parser.hpp"
#include <fstream>
#include <string>


ini_parser::ini_parser()
{
	//массивы для хранения имен секций и их переменных
	variables_str_array = new std::vector<std::map<std::string, std::string>>; //массивом мапов, которые содержат строки
	sections_map = new std::map<std::string, int>; //соответствие названия секции номеру массива variables_str_array

	//если были проблемы при вызовах new
	if ((variables_str_array == nullptr) || (sections_map == nullptr))
	{
		_parser_invalid = true; //есть проблемы с парсером
		return;
	}
}

ini_parser::~ini_parser()
{
	delete variables_str_array;
	delete sections_map;
}

ini_parser::ini_parser(const ini_parser& other) // конструктор копирования
{
	_parser_invalid = other._parser_invalid; //есть ли проблемы с парсером
	_file_read = other._file_read;  //хранит информацию о том, удалось ли считать данные из файла
	_invalid_data = other._invalid_data; //хранит информацию, есть ли в файле некорректная информация
	incorrect_str_num = other.incorrect_str_num;  //номер строки, содержащей некорректный синтаксис
	incorrect_str = other.incorrect_str;  //содержимое строки с некорректным синтаксисом

	//текущая секция для добавления переменных
	current_section_name = other.current_section_name; //имя секции
	current_section_number = other.current_section_number; //номер секции = индекс  этой секции в массиве variables_str_array

	sections_map = new std::map<std::string, int>;
	*sections_map = *(other.sections_map); // new std::map<std::string, int>; //соответствие названия секции номеру массива variables_str_array

	variables_str_array = new std::vector<std::map<std::string, std::string>>; //массивом мапов, которые содержат строки (название переменных и их значения)

	for (int i = 0; i < other.variables_str_array->size(); ++i)
	{
		variables_str_array->push_back((*other.variables_str_array)[i]); //  скопировать map в новый массив
	}
}

ini_parser& ini_parser::operator=(const ini_parser& other) // оператор копирующего присваивания
{
	if (this != &other)
	{
		return *this = ini_parser(other);
	}

	return *this;
}

ini_parser::ini_parser(ini_parser&& other) noexcept	// конструктор перемещения
{
	_parser_invalid = other._parser_invalid; //есть ли проблемы с парсером
	_file_read = other._file_read;  //хранит информацию о том, удалось ли считать данные из файла
	_invalid_data = other._invalid_data; //хранит информацию, есть ли в файле некорректная информация
	incorrect_str_num = other.incorrect_str_num;  //номер строки, содержащей некорректный синтаксис
	incorrect_str = other.incorrect_str;  //содержимое строки с некорректным синтаксисом

	//текущая секция для добавления переменных
	current_section_name = other.current_section_name; //имя секции
	current_section_number = other.current_section_number; //номер секции = индекс  этой секции в массиве variables_str_array

	sections_map = other.sections_map;
	other.sections_map = nullptr;

	variables_str_array = other.variables_str_array;
	other.variables_str_array = nullptr;
}

ini_parser& ini_parser::operator=(ini_parser&& other) noexcept   // оператор перемещающего присваивания
{
	return *this = ini_parser(other);
}

std::string ini_parser::delete_spaces(const std::string& src_str) //удалить пробелы и знаки табуляции в начале строки
{
	if (src_str == "") 	return src_str; //ничего не делать с пустой строкой

	std::string temp_str = "";
	int i = 0;
	bool is_beginning = true;

	while ((src_str[i] != '\n') &&
		(i < src_str.size()))
	{
		if (((src_str[i] == ' ') || (src_str[i] == '\t')) && //знаки пробелов и табуляции 
			is_beginning) //в начале строки
		{
			++i; //пропустить
		}
		else
		{
			temp_str += src_str[i];
			is_beginning = false;
			++i;
		}
	}

	return temp_str;
}

//исследовать строку: возвращаемый кортеж содержит код содержимого, название переменной или секции, значение переменной
std::tuple<string_type, std::string, std::string> ini_parser::research_string(const std::string& src_str)
{
	string_type temp_string_type = string_type::_invalid_;  //строка невалидная
	std::string temp_name = ""; //имя секции или переменной
	std::string temp_value = ""; //значение переменной

	if (src_str == "") //переданная строка пустая
	{
		return std::make_tuple(string_type::_empty_, temp_name, temp_value);
	}

	if (src_str[0] == ';') //переданная строка целиком содержит комментарий
	{
		return std::make_tuple(string_type::_empty_, temp_name, temp_value);
	}

	if (src_str[0] == '[') //переданная строка предположительно содержит название секции
	{
		int end_pos = src_str.find(']'); //номер символа закрывающей скобки

		if ((end_pos == std::string::npos) ||   //закрывающей скобки нет
			(end_pos == 1)) // ||					//имя секции не может быть пустым

		{
			return std::make_tuple(string_type::_invalid_, temp_name, temp_value); //вернуть результат о невалидной строке
		}
		else
		{
			for (int i = 1; i < end_pos; ++i)
			{
				if ((src_str[i] == ' ') || (src_str[i] == '\t') || (src_str[i] == '.')) //название секции не может содержать пробелов, знаков табуляции, точек
				{
					return std::make_tuple(string_type::_invalid_, temp_name, temp_value); //вернуть результат о невалидной строке
				}
				else
				{
					temp_name += src_str[i];
				}
			}

			//после закрывающей скобки могут быть только символы пробелов, табуляции или комментарий
			for (int i = end_pos + 1; i < src_str.size() - 1; ++i)
			{
				if ((src_str[i] != ' ') && (src_str[i] != '\t') && (src_str[i] != ';'))
				{
					return std::make_tuple(string_type::_invalid_, temp_name, temp_value); //вернуть результат о невалидной строке
				}

				if (src_str[i] == ';') //после знака начала комментария можно не проверять
				{
					return std::make_tuple(string_type::_section_, temp_name, temp_value); //вернуть название секции
				}
			}

			//недопустимого содержимого после закрывающей скобки нет, поэтому можно вернуть результат
			return std::make_tuple(string_type::_section_, temp_name, temp_value); //вернуть название секции
		}
	}

	//переданная строка не является пустой, комментарием или названием секции, поэтому может быть только переменной
	int end_pos = src_str.find('='); //номер символа закрывающей скобки

	if (end_pos == std::string::npos)    //знака равенства нет
	{
		return std::make_tuple(string_type::_invalid_, temp_name, temp_value); //вернуть результат о невалидной строке
	}

	for (int i = 0; (i < end_pos) && (src_str[i] != ' ') && (src_str[i] != '\t') && (src_str[i] != '.'); ++i) //до знака равенства или первого знака табуляции или пробела
	{
		temp_name += src_str[i];
	}

	//имя переменной не может быть пустым
	if (temp_name == "")
	{
		return std::make_tuple(string_type::_invalid_, temp_name, temp_value); //вернуть результат о невалидной строке
	}

	// между именем переменной и знаком равенства могут быть только знаки пробелов и табуляций
	for (int i = temp_name.size(); i < end_pos; ++i)
	{
		if ((src_str[i] != ' ') && (src_str[i] != '\t'))
		{
			return std::make_tuple(string_type::_invalid_, temp_name, temp_value); //вернуть результат о невалидной строке
		}
	}

	//после знака равенства до конца строки или до начала комментария - значение переменной
	for (int i = end_pos + 1; i < src_str.size(); ++i)
	{
		if (src_str[i] == ';') //дальше только комментарий
		{
			return std::make_tuple(string_type::_variable_, temp_name, temp_value); //вернуть результат - имя и значение переменной
		}

		temp_value += src_str[i];
	}

	return std::make_tuple(string_type::_variable_, temp_name, temp_value); //вернуть результат - имя и значение переменной
}

std::tuple<std::string, std::string> ini_parser::get_section_variable_names(const std::string& src_str) //из строки запроса получить имя секции и имя переменной
{
	std::string section_name = "";
	std::string variable_name = "";

	std::string temp_str = delete_spaces(src_str);

	int point_pos = temp_str.find('.'); //номер символа точки
	if ((point_pos == std::string::npos) ||    //знака точки нет
		(temp_str.length() < 3)					//в строке запроса не может быть меньше 3 символов a.a
		)
	{
		throw ParserException_incorrect_request();
	}

	char symb; // = ' ';
	int i = 0;

	do
	{
		symb = temp_str[i];
		i++;
		if (symb == ' ') //todo: хорошо бы иметь список разрешенных или запрещенных символов в названиях секций и переменных
		{
			throw ParserException_incorrect_request();
		}

		if (symb == '.')
		{
			break;
		}
		else
		{
			section_name += symb;
		}

	} while (i < temp_str.length());

	if (temp_str.length() < i) //если точка - последний символ в строке, т.е. нет имени переменной
	{
		throw ParserException_incorrect_request();
	}

	do
	{
		symb = temp_str[i];
		if ((symb == '.') || (symb == ' ')) //todo: хорошо бы иметь список разрешенных или запрещенных символов в названиях секций и переменных
		{
			throw ParserException_incorrect_request();
		}

		variable_name += symb;
		++i;

	} while (i < temp_str.length());


	if ((section_name == "") || (variable_name == ""))
	{
		throw ParserException_incorrect_request();
	}

	return std::make_tuple(section_name, variable_name); //вернуть результат - имена секции и переменной
}

std::string ini_parser::get_section_from_request(const std::string& request_str) //получить из строки запроса название секции
{
	std::string section_name = "";
	std::string variable_name = "";

	std::tie(section_name, variable_name) = get_section_variable_names(request_str); //может выбросить исключение ParserException_incorrect_request()

	return section_name;
}


void ini_parser::check_parser() //проверяет, нет ли в парсере проблем и выбрасывает исключение
{
	//bool _file_read = false; //хранит информацию о том, удалось ли считать данные из файла
	if (!_file_read)
	{
		throw ParserException_no_file();
	}

	//bool _parser_invalid = false; //есть проблемы с парсером
	if ((_parser_invalid) || (sections_map == nullptr) || (variables_str_array == nullptr))
	{
		throw ParserException_error();
	}

	//bool invalid_data = false; //хранит информацию, есть ли в файле некорректная информация
	if (_invalid_data)
	{
		throw ParserException_incorrect_data();
	}


}

void ini_parser::print_incorrect_info()	//вывести информацию о некорректных данных в файле
{
	std::cout << "В файле содержится некорректная информация.\n";
	std::cout << "Номер строки с ошибкой: " << incorrect_str_num << "\n";
	std::cout << "Строка: \"" << incorrect_str << "\"\n";
}

bool ini_parser::print_all_sections()
{
	bool print_result = true;

	try
	{
		check_parser();  //проверяет, нет ли в парсере проблем и выбрасывает исключение
	}
	catch (const ParserException_incorrect_data& ex)
	{
		print_result = false;
	}

	if (sections_map->size() > 0)
	{
		std::cout << "В файле имеются следующие секции:\n";
	}

	for (auto& item : *sections_map) //перебрать все элементы мапа
	{
		std::cout << item.first << "\n";
	}

	return print_result;
}

bool ini_parser::print_all_sections_info() //вывести на экран названия всех секций и их переменные
{
	std::cout << "\nПарсер содержит следующие данные: \n";

	bool print_result = true;

	try
	{
		check_parser();  //проверяет, нет ли в парсере проблем и выбрасывает исключение
	}
	catch (const ParserException_incorrect_data& ex)
	{
		print_result = false;	//если в файле есть данные - попробовать их вывести, но предупредить о возможной некорректности данных	
	}

	for (int i = 0; i < sections_map->size(); ++i)
	{
		std::cout << i << ": " << get_section_name(i) << "\n"; //получить имя секции по ее номеру
	}

	std::cout << "Всего секций = " << variables_str_array->size() << "\n\n";

	for (int i = 0; i < variables_str_array->size(); ++i)
	{
		std::cout << "Размер секции " << i << " (" << get_section_name(i) << ") " << " = " << (*variables_str_array)[i].size() << "\n";

		std::cout << "Список переменных:\n";
		for (auto& item : (*variables_str_array)[i])
		{
			std::cout << item.first << " = " << item.second << "\n";
		}
		std::cout << "\n";
	}

	std::cout << "***************************" << "\n";
	return print_result;
}

bool ini_parser::print_all_variables(const std::string& section_name) //вывести на экран все переменные в секции
{
	bool print_result = true;

	try
	{
		check_parser();  //проверяет, нет ли в парсере проблем и выбрасывает исключение
	}
	catch (const ParserException_incorrect_data& ex)
	{
		print_result = false;
	}


	int section_index = get_section_number(section_name); //получить номер секции по ее имени
	if (section_index < 0)
	{
		//бросить исключение - секция не существует
		throw(ParserException_no_section());
	}
	else
	{
		if ((*variables_str_array)[section_index].size() > 0)
		{
			std::cout << "В секции " << section_name << " имеются следующие переменные:\n";
			for (auto& item : (*variables_str_array)[section_index]) //перебрать все элементы мапа
			{
				std::cout << item.first << " = " << item.second << "\n";
			}
		}
	}

	return print_result;
}

std::string ini_parser::get_section_name(const int section_index) //получить имя секции по ее номеру
{
	for (auto& item : *sections_map) //перебрать все элементы мапа
	{
		if (item.second == section_index) //если значение равно индексу
		{
			return item.first; //вернуть название
		}
	}

	return "";
}

int ini_parser::get_section_number(const std::string& section_name) //получить номер секции по ее имени
{
	//если такого ключа нет в мапе
	if ((*sections_map).find(section_name) == (*sections_map).end())
	{
		return -1;
	}

	int temp = (*sections_map)[section_name];
	return temp;
}

std::string ini_parser::get_variable_value(const int section_index, const std::string& variable_name) //получить значение переменной по имени секции и имени переменной
{
	if (section_index < 0)
	{
		throw ParserException_no_section(); //в функцию передана -1 - такой секции нет
	}

	if (section_index >= variables_str_array->size()) //проблема либо в коде парсера, либо в использовании функции get_variable_value
	{
		throw ParserException_error();
	}

	//такой переменной нет в секции
	if ((*variables_str_array)[section_index].find(variable_name) == (*variables_str_array)[section_index].end())
	{
		throw ParserException_no_variable();
		return "";
	}

	return ((*variables_str_array)[section_index])[variable_name];
}

void ini_parser::fill_parser(const std::string& file_name)
{
	//открыть файл
	std::ifstream fin(file_name);
	if (!fin.is_open())
	{
		_parser_invalid = true;
		return;
	}

	//построчное чтение из файла
	std::string temp_str;
	int lines_count = 0; //счетчик считанных из файла строк
	int sections_count = 0;  //счетчик секций

	while (getline(fin, temp_str))  // getline забирает всю строку до знака перевода строки
	{
		++lines_count;
		std::string temp1 = delete_spaces(temp_str); //удалить пробелы и знаки табуляции из начала строки

		string_type string_type_ = string_type::_invalid_;  //строка невалидная
		std::string name = ""; //имя секции или переменной
		std::string value = ""; //значение переменной

		std::tie(string_type_, name, value) = research_string(temp1); //исследовать строку и понять, что с ней делать дальше
		std::map<std::string, int>::iterator it_name; //итератор нельзя создать внутри switch-case (интересно, почему)

		switch (string_type_) //string_type_ после исследования строки research_string содержит тип строки
		{
		case string_type::_section_: //название секции

			current_section_name = name;		//текущая секция для добавления переменных
			it_name = sections_map->find(name); //есть ли уже такая секция в массиве парсера

			if (it_name == sections_map->end()) //если такой секции еще нет
			{
				//добавить ее
				(*sections_map)[name] = sections_count; //добавить секцию в мап, соединяющий номера и названия секций
				current_section_number = sections_count;
				++sections_count;

				std::map<std::string, std::string> new_temp_map{}; //временный пустой мап
				variables_str_array->push_back(new_temp_map); //добавить пустой map в массив
			}
			else //такая секция уже была
			{
				current_section_number = (*sections_map)[name]; //индекс секции в массиве variables_str_array
			}
			break;

		case string_type::_variable_: //переменная

			if (current_section_number < 0) //еще не было добавлено ни одной секции, а все переменные должны принадлежать какой-либо секции
			{
				incorrect_str_num = lines_count;  //номер строки, содержащей некорректный синтаксис
				incorrect_str = temp_str;  //содержимое строки с некорректным синтаксисом
			}
			else
			{
				//добавить или обновить переменную в мапе соответствующей секции
				(*variables_str_array)[current_section_number][name] = value;
			}

			break;

		case string_type::_empty_:	   //пустая строка или комментарий			
			break;

		case string_type::_invalid_:   //строка невалидная
			incorrect_str_num = lines_count;  //номер строки, содержащей некорректный синтаксис
			incorrect_str = temp_str;  //содержимое строки с некорректным синтаксисом

			break;
		}

		if (incorrect_str_num != 0) //не имеет смысла продолжать - в файле некорректные данные
		{
			_invalid_data = true;
			break; //прервать чтение данных из файла
		}

	}

	fin.close();
	_file_read = true; //хранит информацию о том, удалось ли считать данные из файла

	if (variables_str_array->size() != sections_map->size())
	{
		_parser_invalid = true;
	}
}