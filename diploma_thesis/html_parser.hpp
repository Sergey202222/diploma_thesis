#pragma once

#include <iostream>
#include <regex>
#include <set>
#include <map>

class html_parser
{
private:
	std::string complete_url(const std::string& in_url, const std::string& url_base);
	std::string get_base_path(const std::string& in_str);
	bool check_this_host_only(const std::string& host_url, const std::string& url_str, bool this_host_only); //проверка, является ли урл требуемым хостом	

public:
	int max_word_len = 32; //максимальная длина слова для добавления в базу данных
	int min_word_len = 3;  //минимальная длина слова для добавления в базу данных

	std::string get_base_host(const std::string& url_str); //получить хост из строки	
	std::string clear_tags(const std::string& html_body_str);	//очистить строку от тегов, в том числе все до тега <body>

	std::set<std::string> get_urls_from_html(const std::string& html_body_str, const std::string& base_str, bool this_host_only, std::string host_url); //получить список новых урлов
	std::map<std::string, unsigned  int> collect_words(const std::string& text_str); //получить мап слов 
};