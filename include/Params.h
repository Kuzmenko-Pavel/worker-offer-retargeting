#ifndef PARAMS_H
#define PARAMS_H

#include <sstream>
#include <string>
#include <boost/date_time.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/regex/icu.hpp>
#include <vector>
#include <map>
#include "json.h"

/** \brief Параметры, которые определяют показ рекламы */
class Params
{
public:
    std::string cookie_id_;
    unsigned long long key_long;
    boost::posix_time::ptime time_;
    std::string get_;
    std::string post_;
    nlohmann::json params_;
    nlohmann::json informer_;
    

    Params();
    Params &parse();
    Params &cookie_id(const std::string &cookie_id);
    Params &json(const std::string &json);
    Params &get(const std::string &get);
    Params &post(const std::string &post);

    std::string getIP() const;
    std::string getCookieId() const;
    std::string getUserKey() const;
    std::string getCampaigns() const;
    std::string getExclude() const;
    unsigned int getCapacity() const;
    long long  getInformerIdInt() const;
    unsigned long long getUserKeyLong() const;
    std::string getInformerId() const;
    boost::posix_time::ptime getTime() const;
    bool isTestMode() const;
    std::map<const unsigned long,int> getRetargetingOffersDayMap();
    std::vector<std::string> getRetargetingOffers();
    std::map<const unsigned long,int> getRetargetingViewOffers();

private:
    boost::u32regex replaceSymbol;
    nlohmann::json json_;
    unsigned int capacity;
    std::string informer_id;
    long long informer_id_int;
    std::vector<std::string> exclude;
    std::vector<std::string> campaign;
    bool test_mode;
    std::map<const unsigned long,int> retargeting_offers_day_;
    std::vector<std::string> retargeting_offers_;
    std::map<const unsigned long,int> retargeting_view_offers_;
};

#endif // PARAMS_H
