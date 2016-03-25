#ifndef URLPARSER_H
#define URLPARSER_H

#include <map>
#include <string>

/** \brief Парсит URL */
// TODO: Поддержка UrlEncoded значений параметров
class UrlParser
{
public:
    /** \brief Парсит строку url */
    UrlParser(const std::string &url);

    /** \brief Возвращает карту параметров (key => value) */
    std::map<std::string, std::string> params() const
    {
        return params_;
    }

    /** \brief Возвращает параметр par или пустую строку, если параметр не найден */
    std::string param(const std::string &par) const;

private:
    std::string url_;
    std::map<std::string, std::string> params_;

    void parse();
    std::string percent_decode(const std::string &str) const;
    bool is_hex_digit(char hex_digit) const;
    int hex_digit_to_int(char hex_digit) const;
};

#endif // URLPARSER_H
