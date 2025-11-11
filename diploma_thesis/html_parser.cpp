#include "html_parser.hpp"

std::string html_parser::complete_url(const std::string & in_url, const std::string & url_base)
{
    if (in_url.empty())
    {
        return  url_base;
    }

    std::string res_url = in_url;
    std::regex_replace(res_url, std::regex("/$"), "");

    auto pos = res_url.find("https://");
    if (pos == 0) return res_url;

    pos = res_url.find("http://");
    if (pos == 0) return res_url;

    pos = res_url.find("www.");
    if (pos == 0) return res_url;

    pos = res_url.find("/");
    if (pos == 0)
    {
        res_url.erase(0, 1);
    }


    res_url = url_base + "/" + res_url;

    return res_url;
}

std::set<std::string> html_parser::get_urls_from_html(const std::string& html_body_str, const std::string& base_str, bool this_host_only, std::string host_url)
{
    std::set<std::string> urls_set;
    std::string s2 = html_body_str;

    host_url = get_base_host(host_url);

    //поиск ссылок <a href>
    std::smatch res1;
    std::regex r1("<a (.?[^>]*?)href=\"(.*?)\"");
    while (regex_search(s2, res1, r1))
    {
        std::string find_str = res1.str();
        std::string url_str = std::regex_replace(find_str, std::regex("<a (.?[^>]*?)href=\""), ""); //удалить все атрибуты между именем a и атрибутом href
        url_str = std::regex_replace(url_str, std::regex("\""), ""); //удалить все кавычки
        url_str = std::regex_replace(url_str, std::regex("/$"), ""); //удалить слеш в конце строки       
        std::string new_base_path = get_base_path(url_str); //найти базовую часть пути  


        std::string suf_str = url_str.erase(0, new_base_path.size()); //найти им€ страницы или скрипта       
        std::string final_url = complete_url(new_base_path, base_str) + suf_str; //финальный вид урла

        if (check_this_host_only(host_url, final_url, this_host_only))
        {
            urls_set.insert(final_url);
        }

        s2 = res1.suffix(); //продолжить поиск в оставшейс€ части      
    };
    return urls_set;
}

std::string html_parser::clear_tags(const std::string& html_body_str)
{

    std::string s2 = html_body_str;


    s2 = std::regex_replace(s2, std::regex("[\n\t]"), " ");
    //s2 = std::regex_replace(s2, std::regex("\n"), " ");
    //s2 = std::regex_replace(s2, std::regex("\t"), " ");
    s2 = std::regex_replace(s2, std::regex(">"), "> ");
    s2 = std::regex_replace(s2, std::regex("< /"), "</");
    s2 = std::regex_replace(s2, std::regex("</ "), "</");
    s2 = std::regex_replace(s2, std::regex("< {1,}"), "<");
    s2 = std::regex_replace(s2, std::regex(" {1,}>"), ">");

    //std::cout << "s2" << s2 << "\n";

    std::string regex_str;

    //вытащить текст из title
    std::string res_str;
    std::smatch res1;
    regex_str = "<title>(.?)[^<]*";
    if (regex_search(s2, res1, std::regex(regex_str)))
    {
        res_str = res1.str();
        res_str = std::regex_replace(res_str, std::regex("<title>"), "");

        // "openssl - Conan 2.0: C and C++ Open Source Package Manager"
       //  s2 = std::regex_replace(s2, std::regex("openssl - Conan 2.0: C and C Open Source Package Manager"), "");
        // s2 = std::regex_replace(s2, std::regex(res_str), "");
    }

    regex_str = ("^.+?(<body)");//удалить все до тега body
    s2 = std::regex_replace(s2, std::regex(regex_str), "");

    regex_str = "<(.?)[^>][^<]*>";
    while (regex_search(s2, res1, std::regex(regex_str)))
    {
        s2 = std::regex_replace(s2, std::regex(regex_str), "");
    };



    regex_str = ("(u003c)");//спецсимволы
    s2 = std::regex_replace(s2, std::regex(regex_str), " ");
    regex_str = ("(u003e)");//спецсимволы
    s2 = std::regex_replace(s2, std::regex(regex_str), " ");



    res_str += " ";
    res_str += s2;

    // res_str = "<h1>Example Domain</h1> * + = || _ -  ~# $ % ^ &  \ < p >\ Thi//s d,om;ain is for /use in illu, strat: / ive exa:m\"ples\" : in doc& 'ume'? ? (nts.You) may use [this  ;  {;domain;} in literature without prior coordination or asking for permission. < / p>";
    // std::cout << "______res_str1 = " << res_str << "\n\n";

     //не удаетс€ экранировать символы ] и \ - их добавление в рег. выражение вызывает падение программы, хот€ в тестах на сторонних сайтах выражени€ считаютс€ валидными
     // res_str = std::regex_replace(res_str, std::regex("[\.,:;!~=%&#\^\|\$\[\]\\/\?\<>\(\)\{\}\"'\*\+_\-]"), " "); //убрать знаки препинани€ и спец символы

    res_str = std::regex_replace(res_str, std::regex("[\.,:;@!~=%&#\^\|\$\[\\/\?\<>\(\)\{\}\"'\*\+_\-]"), " "); //убрать знаки препинани€ и спец символы
    res_str = std::regex_replace(res_str, std::regex(" {2,}"), " "); //убрать лишние пробелы    

    //функцией regex_replace не получаетс€ удалить знаки \ и ], поэтому их удал€ю отдельно
    int pos;
    while (!((pos = res_str.find("\\")) == std::string::npos))
    {
        res_str.erase(pos, 1);
    }

    while (!((pos = res_str.find("]")) == std::string::npos))
    {
        res_str.erase(pos, 1);
    }


    //все строчные
    std::transform(res_str.begin(), res_str.end(), res_str.begin(),
        [](unsigned char c) { return std::tolower(c); });


    return res_str;
}


std::string html_parser::get_base_host(const std::string& url_str)
{
    std::string res_str = url_str;

    std::string http_prefix = "https://";

    if (res_str.find(http_prefix) == std::string::npos)
    {
        http_prefix = "http://";
        if (res_str.find(http_prefix) == std::string::npos)
        {
            http_prefix = "";
        }
    }

    res_str.erase(0, http_prefix.size());

    auto pos = res_str.find("/");
    if (pos != std::string::npos)
    {
        int num = res_str.size() - pos;
        res_str.erase(pos, num);
    }

    return http_prefix + res_str;
}

std::string html_parser::get_base_path(const std::string& in_str)
{
    std::string res_str = in_str;
    res_str = std::regex_replace(res_str, std::regex("/$"), "");

    std::string http_prefix = "https://";
    if (res_str.find(http_prefix) == std::string::npos)
    {
        http_prefix = "http://";
        if (res_str.find(http_prefix) == std::string::npos)
        {
            http_prefix = "";
        }
    }

    res_str.erase(0, http_prefix.size());

    std::smatch res;
    std::regex r("/(.[^/]*)$");
    if (regex_search(res_str, res, r))
    {
        std::string find_str = res.str();
        auto n_pos = res_str.find(find_str);
        if (find_str.find(".") != std::string::npos)
        {
            res_str.erase(n_pos, find_str.size());
        }
    };

    res_str = std::regex_replace(res_str, std::regex("/{2,}"), "/");

    return http_prefix + res_str;
}

std::map<std::string, unsigned  int> html_parser::collect_words(const std::string& text_str)
{
    std::string search_str = text_str;

    //извлечь следующее слово
    std::smatch res;
    std::regex r("(.[^ ]*)");

    std::map<std::string, unsigned  int> words_map;

    while (regex_search(search_str, res, r))
    {
        std::string find_str = res.str();
        find_str = std::regex_replace(find_str, std::regex(" "), "");

        int len = find_str.size();
        if ((len >= min_word_len) && (len <= max_word_len))
        {
            auto word_pair = words_map.find(find_str);
            if (word_pair != words_map.end())
            {
                // int words_count = words_map[find_str] + 1;
                words_map[find_str] = words_map[find_str] + 1;//words_count;                   
            }
            else
            {
                words_map[find_str] = 1;
            }
        }

        search_str = res.suffix();
    };

    return words_map;
}

bool html_parser::check_this_host_only(const std::string& host_url, const std::string& url_str, bool this_host_only) //проверка, €вл€етс€ ли урл требуемым хостом
{
    if (!(this_host_only)) return true; //не нужна проверка

    if (url_str.find(host_url) != 0)
    {
        return false;
    }
    else
        return true;

}