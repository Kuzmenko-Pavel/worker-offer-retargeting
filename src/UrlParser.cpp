//#include <iostream>
#include <sstream>

#include "UrlParser.h"


UrlParser::UrlParser(const std::string &url)
    : url_(url)
{
    parse();
    //parseReferrer();//добавлено RealInvest Soft
}

/** Возвращает параметр par или пустую строку, если параметр не найден */
std::string UrlParser::param(const std::string &par) const
{
    std::map<std::string,std::string>::const_iterator it = params_.find(par);
    if (it != params_.end())
        return it->second;
    else
        return "";
}


/** Разбор URL-строки */
void UrlParser::parse()
{
    // Ищем первый вопросительный знак
    std::string::size_type pos = url_.find_first_of('?');
    if (pos == std::string::npos)
        pos = -1;

    while (pos < url_.size() || pos == (std::string::size_type)-1)
    {
        // Выделяем название параметра
        std::string::size_type end = url_.find_first_of('=', ++pos);
        if (end == std::string::npos) return;
        std::string param_name = url_.substr(pos, end - pos);
        pos = end;

        // Выделяем значение параметра
        end = url_.find_first_of('&', ++pos);
        if (end == std::string::npos)
            end = url_.size();
        std::string param_value = url_.substr(pos, end - pos);
        pos = end;
        params_[param_name] = percent_decode(param_value);
    }
}

/** Декодирует строку str из процентного представления */
std::string UrlParser::percent_decode(const std::string &str) const
{
    enum State
    {
        General,
        FirstPercentEncodedDigit,
        SecondPercentEncodedDigit
    } state = General;

    char first_char = '\0',
         second_char = '\0';
    std::stringstream result;

    for (std::string::const_iterator it = str.begin(); it != str.end(); it++)
    {
        switch (state)
        {

        case General:
            if (*it != '%')
                result << *it;
            else
            {
                state = FirstPercentEncodedDigit;
                first_char = second_char = '\0';
            }
            break;

        case FirstPercentEncodedDigit:
            first_char = *it;
            if (is_hex_digit(first_char))
            {
                state = SecondPercentEncodedDigit;
            }
            else
            {
                result << "%" << first_char;
                state = General;
            }
            break;

        case SecondPercentEncodedDigit:
            second_char = *it;
            if (is_hex_digit(second_char))
            {

                result << char(
                           (hex_digit_to_int(first_char) << 4) |
                           hex_digit_to_int(second_char));
                state = General;
            }
            else
            {
                result << "%" << first_char << second_char;
                state = General;
            }
            break;
        }
    }
    return result.str();
}


/** Возвращает true, если \a hex_digit является шестнадцатиричным символом */
bool UrlParser::is_hex_digit(char hex_digit) const
{
    switch (hex_digit)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 'a':
    case 'A':
    case 'b':
    case 'B':
    case 'c':
    case 'C':
    case 'd':
    case 'D':
    case 'e':
    case 'E':
    case 'f':
    case 'F':
        return true;
    default:
        return false;
    }
}

/** Возвращает число от 0 до 15, соответствующее hex_digit.

    Если hex_digit не является шестнадцатиричным символом, возвращат 0 */
int UrlParser::hex_digit_to_int(char hex_digit) const
{
    switch (hex_digit)
    {
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    case 'a':
    case 'A':
        return 10;
    case 'b':
    case 'B':
        return 11;
    case 'c':
    case 'C':
        return 12;
    case 'd':
    case 'D':
        return 13;
    case 'e':
    case 'E':
        return 14;
    case 'f':
    case 'F':
        return 15;
    default:
        return 0;
    }
}
