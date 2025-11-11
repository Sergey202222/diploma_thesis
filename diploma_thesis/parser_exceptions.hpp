#pragma once
#include <iostream>

class ParserException_error : public std::exception
{
public:
	const char* what() const override
	{
		return "Ошибка парсера";
	}
};

class ParserException_no_file : public std::exception
{
public:
	const char* what() const override
	{
		return "Не получилось считать данные из файла";
	}
};

class ParserException_incorrect_data : public std::exception
{
public:
	const char* what() const override
	{
		return "В файле содержатся некорректные данные. Результаты запроса могут быть некорректными или неполными.";
	}
};

class ParserException_no_section : public std::exception
{
public:
	const char* what() const override
	{
		return "Запрашиваемая секция не существует";
	}
};

class ParserException_no_variable : public std::exception
{
public:
	const char* what() const override
	{
		return "В секции нет запрашиваемой переменной";
	}
};

class ParserException_incorrect_request : public std::exception
{
public:
	const char* what() const override
	{
		return "Ошибка в строке запроса";
	}
};

class ParserException_type_error : public std::exception
{
public:
	const char* what() const override
	{
		return "Неверный тип переменной в запросе";
	}
};