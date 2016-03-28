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


    if (params_.count("retargetingOffer") && params_["retargetingOffer"].is_string())
    {
        std::string pla = params_["retargetingOffer"];
        replaceSymbol = boost::make_u32regex("([A-Za-z\\.:\\-\\s_]+;)|([A-Za-z\\.:\\-\\s_]+)");
        pla = boost::u32regex_replace(pla,replaceSymbol,"");
        boost::trim(pla);
        if(pla != "")
        {
            boost::split(campaign, pla, boost::is_any_of(";"));
        }
    }
    if (params_.count("retargeting_exclude") && params_["retargeting_exclude"].is_string())
    {
        std::string exc = params_["retargeting_exclude"];
        replaceSymbol = boost::make_u32regex("([A-Za-z\\.:\\-\\s_]+;)|([A-Za-z\\.:\\-\\s_]+)");
        exc = boost::u32regex_replace(exc,replaceSymbol,"");
        boost::trim(exc);
        if(exc != "")
        {
            boost::split(exclude, exc, boost::is_any_of(";"));
        }
    }
    if (informer_.count("retargeting_capacity") && informer_["retargeting_capacity"].is_number())
    {
        capacity = informer_["retargeting_capacity"];
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
    if (params_.count("retargeting") && params_["retargeting"].is_string())
    {
        std::string retargeting = params_["retargeting"];
        if(retargeting != "")
        {
            boost::algorithm::to_lower(retargeting);
            boost::split(retargeting_offers_, retargeting, boost::is_any_of(";"));
            if (retargeting_offers_.size()> 50)
            {
                retargeting_offers_.erase(retargeting_offers_.begin()+49, retargeting_offers_.end());
            }
        }
        for (auto i=retargeting_offers_.begin(); i != retargeting_offers_.end() ; ++i)
        {
            std::vector<std::string> par;
            boost::split(par, *i, boost::is_any_of("~"));
            if (!par.empty() && par.size() >= 4)
            {
                if (!par[0].empty())
                {
                    try
                    {
                        retargeting_offers_day_.insert(std::pair<const unsigned long,int>(stoul(par[0]),stoi(par[3])));
                    }
                    catch (std::exception const &ex)
                    {
                        Log::err("exception %s: name: %s while processing etargeting_offers: %s", typeid(ex).name(), ex.what(), (*i).c_str());
                    }
                }
            }
        }
    }
    if (params_.count("retargeting_view") && params_["retargeting_view"].is_string())
    {
        std::vector<std::string> retargeting_view_offers;
        std::string retargeting_view = params_["retargeting_view"];
        if(retargeting_view != "")
        {
            boost::split(retargeting_view_offers, retargeting_view, boost::is_any_of(";"));
        }
        for (unsigned i=0; i<retargeting_view_offers.size() ; i++)
        {
            std::vector<std::string> par;
            boost::split(par, retargeting_view_offers[i], boost::is_any_of("~"));
            if (!par.empty() && par.size() >= 2)
            {
                try
                {
                    retargeting_view_offers_.insert(std::pair<const unsigned long,int>(stol(par[0]),stoi(par[1])));
                }
                catch (std::exception const &ex)
                {
                    Log::err("exception %s: name: %s while processing retargeting_view_offers: %s", typeid(ex).name(), ex.what(), retargeting_view.c_str());
                }
            }
        }
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
    return boost::algorithm::join(campaign, ", ");
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
std::map<const unsigned long,int> Params::getRetargetingOffersDayMap()
{
    return retargeting_offers_day_;
}

std::vector<std::string> Params::getRetargetingOffers()
{
    return retargeting_offers_;
}
std::map<const unsigned long,int> Params::getRetargetingViewOffers()
{
    return retargeting_view_offers_;
}
