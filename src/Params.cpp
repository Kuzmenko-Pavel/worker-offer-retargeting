#include <sstream>

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/regex/icu.hpp>
#include <boost/date_time.hpp>

#include <string>

#include "Params.h"
#include "Log.h"
#include <map>


Params::Params()
{
    time_ = boost::posix_time::second_clock::local_time();
}

std::string time_t_to_string(time_t t)
{
    std::stringstream sstr;
    sstr << t;
    return sstr.str();
}

Params &Params::cookie_id(const std::string &cookie_id)
{
    if(cookie_id.empty())
    {
        cookie_id_ = time_t_to_string(time(NULL));
    }
    else
    {
        cookie_id_ = cookie_id;
        replaceSymbol = boost::make_u32regex("[^0-9]");
        cookie_id_ = boost::u32regex_replace(cookie_id_ ,replaceSymbol,"");
    }
    boost::trim(cookie_id_);
    key_long = atol(cookie_id_.c_str());

    return *this;
}

Params &Params::json(const std::string &json)
{
    try
    {
        json_ = nlohmann::json::parse(json);
    }
    catch (std::exception const &ex)
    {
        #ifdef DEBUG
            printf("%s\n",json.c_str());
        #endif // DEBUG
        Log::err("exception %s: name: %s while parse post", typeid(ex).name(), ex.what());
    }
    return *this;
}
Params &Params::get(const std::string &get)
{
    get_ = get;
    return *this;
}
Params &Params::post(const std::string &post)
{
    post_ = post;
    return *this;
}
Params &Params::parse()
{
    try
    {
        if (json_["params"].is_object())
        {
            params_ = json_["params"];
        }
    }
    catch (std::exception const &ex)
    {
        Log::err("exception %s: name: %s while create json params", typeid(ex).name(), ex.what());
    }
    try
    {
        if (json_["informer"].is_object())
        {
            informer_ = json_["informer"];
        }
    }
    catch (std::exception const &ex)
    {
        Log::err("exception %s: name: %s while create json informer", typeid(ex).name(), ex.what());
    }


    if (params_.count("social") && params_["social"].is_string())
    {
        std::string soc = params_["social"];
        replaceSymbol = boost::make_u32regex("([A-Za-z\\.:\\-\\s_]+;)|([A-Za-z\\.:\\-\\s_]+)");
        soc = boost::u32regex_replace(soc,replaceSymbol,"");
        boost::trim(soc);
        if(soc != "")
        {
            boost::split(social, soc, boost::is_any_of(";"));
        }
    }

    if (params_.count("place") && params_["place"].is_string())
    {
        std::string pla = params_["place"];
        replaceSymbol = boost::make_u32regex("([A-Za-z\\.:\\-\\s_]+;)|([A-Za-z\\.:\\-\\s_]+)");
        pla = boost::u32regex_replace(pla,replaceSymbol,"");
        boost::trim(pla);
        if(pla != "")
        {
            boost::split(place, pla, boost::is_any_of(";"));
        }
    }
    if (params_.count("exclude") && params_["exclude"].is_string())
    {
        std::string exc = params_["exclude"];
        replaceSymbol = boost::make_u32regex("([A-Za-z\\.:\\-\\s_]+;)|([A-Za-z\\.:\\-\\s_]+)");
        exc = boost::u32regex_replace(exc,replaceSymbol,"");
        boost::trim(exc);
        if(exc != "")
        {
            boost::split(exclude, exc, boost::is_any_of(";"));
        }
    }
    if (params_.count("capacity") && params_["capacity"].is_number())
    {
        capacity = params_["capacity"];
    }
    if (params_.count("informer_id") && params_["informer_id"].is_string())
    {
        informer_id = params_["informer_id"];
    }

    if (params_.count("informer_id_int") && params_["informer_id_int"].is_number())
    {
        informer_id_int = params_["informer_id_int"];
    }
    if (params_.count("test") && params_["test"].is_boolean())
    {
        test_mode = params_["test"];
    }

    return *this;
}
std::string Params::getCookieId() const
{
    return cookie_id_;
}

std::string Params::getUserKey() const
{
    return cookie_id_;
}

unsigned long long Params::getUserKeyLong() const
{
    return key_long;
}
boost::posix_time::ptime Params::getTime() const
{
    return time_;
}
std::string Params::getCampaigns() const
{
    return boost::algorithm::join(place, ", ");
}
std::string Params::getSocialCampaigns() const
{
    return boost::algorithm::join(social, ", ");
}
std::string Params::getExclude() const
{
    return boost::algorithm::join(exclude, ", ");
}
bool Params::isTestMode() const
{
    return test_mode;
}
long long Params::getInformerIdInt() const
{
    return informer_id_int;
}
unsigned int Params::getCapacity() const
{
    return capacity;
}
