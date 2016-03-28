#include <boost/foreach.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <map>

#include <ctime>
#include <cstdlib>
#include <chrono>

#include "../config.h"

#include "Config.h"
#include "Core.h"
#include "DB.h"
#include "base64.h"

Core::Core()
{
    tid = pthread_self();
    std::clog<<"["<<tid<<"]core start"<<std::endl;
}
//-------------------------------------------------------------------------------------------------------------------
Core::~Core()
{
}
//-------------------------------------------------------------------------------------------------------------------
std::string Core::Process(Params *prms)
{
    #ifdef DEBUG
        auto start = std::chrono::high_resolution_clock::now();
        printf("%s\n","/////////////////////////////////////////////////////////////////////////");
    #endif // DEBUG
    startCoreTime = boost::posix_time::microsec_clock::local_time();

    params = prms;
    getOffers(items, params);
    RISAlgorithm(items);
    resultHtml();

    endCoreTime = boost::posix_time::microsec_clock::local_time();
    #ifdef DEBUG
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        printf("Time %s taken: %lld \n", __func__,  microseconds);
        printf("%s\n","/////////////////////////////////////////////////////////////////////////");
    #endif // DEBUG

    return retHtml;
}
//-------------------------------------------------------------------------------------------------------------------
void Core::log()
{
    if(cfg->toLog())
    {
        std::clog<<"["<<tid<<"]";
    }
    if(cfg->logCoreTime)
    {
        std::clog<<" core time:"<< boost::posix_time::to_simple_string(endCoreTime - startCoreTime);
    }

    if(cfg->logOutPutSize)
        std::clog<<" out:"<<vResult.size();

}
//-------------------------------------------------------------------------------------------------------------------
void Core::ProcessSaveResults()
{
    request_processed_++;

    log();

    for (Offer::it o = items.begin(); o != items.end(); ++o)
    {
        if(o->second)
            delete o->second;
    }
    //clear all offers map
    items.clear();
    vResult.clear();
    OutPutCampaignSet.clear();
    OutPutOfferSet.clear();

    if(cfg->toLog())
        std::clog<<std::endl;
}
//-------------------------------------------------------------------------------------------------------------------
nlohmann::json Core::OffersToJson(Offer::Vector &items)
{
    #ifdef DEBUG
        auto start = std::chrono::high_resolution_clock::now();
        printf("%s\n","/////////////////////////////////////////////////////////////////////////");
    #endif // DEBUG
    nlohmann::json j;
    for (auto it = items.begin(); it != items.end(); ++it)
    {
        j.push_back((*it)->toJson());
    }
    #ifdef DEBUG
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        printf("Time %s taken: %lld \n", __func__,  microseconds);
        printf("%s\n","/////////////////////////////////////////////////////////////////////////");
    #endif // DEBUG
    return j;
}
//-------------------------------------------------------------------------------------------------------------------
void Core::resultHtml()
{
    #ifdef DEBUG
        auto start = std::chrono::high_resolution_clock::now();
        printf("%s\n","/////////////////////////////////////////////////////////////////////////");
    #endif // DEBUG
    nlohmann::json j;
    j["retargering"] = OffersToJson(vResult);
    retHtml = j.dump();
    #ifdef DEBUG
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        printf("Time %s taken: %lld \n", __func__,  microseconds);
        printf("%s\n","/////////////////////////////////////////////////////////////////////////");
    #endif // DEBUG
}
//-------------------------------------------------------------------------------------------------------------------
void Core::RISAlgorithm(const Offer::Map &items)
{
    #ifdef DEBUG
        auto start = std::chrono::high_resolution_clock::now();
        printf("%s\n","------------------------------------------------------------------");
        printf("RIS offers %lu \n",items.size());
    #endif // DEBUG
    Offer::MapRate result;
    std::map<const unsigned long, int> retargeting_view_offers = params->getRetargetingViewOffers();

    if( items.size() == 0)
    {
        std::clog<<"["<<tid<<"]"<<typeid(this).name()<<"::"<<__func__<< "error items size: 0"<<std::endl;
        #ifdef DEBUG
            auto elapsed = std::chrono::high_resolution_clock::now() - start;
            long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
            printf("Time %s taken return items.size() == 0: %lld \n", __func__,  microseconds);
            printf("%s\n","------------------------------------------------------------------");
        #endif // DEBUG
        return;
    }

    //sort by rating
    for(auto i = items.begin(); i != items.end(); i++)
    {
        if((*i).second)
        {
                if(!(*i).second->social)
                {
                    result.insert(Offer::PairRate((*i).second->rating, (*i).second));
                }
        }
    }
    for(auto i = items.begin(); i != items.end(); i++)
    {
        if((*i).second)
        {
            if((*i).second->social)
            {
                result.insert(Offer::PairRate((*i).second->rating, (*i).second));
            }
        }
    }
    unsigned int passage;
    passage = 0;
    while (passage <= retargeting_view_offers.size())
    {
      for(auto p = result.begin(); p != result.end(); ++p)
        {
            std::map<const unsigned long, int>::iterator it= retargeting_view_offers.find((*p).second->id_int);
            if( it != retargeting_view_offers.end() )
            {
                if(it->second > passage)
                {
                    continue;
                }
            }
            if(OutPutCampaignSet.count((*p).second->campaign_id) < (*p).second->unique_by_campaign
                    && OutPutOfferSet.count((*p).second->id_int) == 0 && !(*p).second->is_recommended)
            { 
                if(vResult.size() >= params->getCapacity())
                    break;
                
                vResult.push_back((*p).second);
                OutPutOfferSet.insert((*p).second->id_int);
                OutPutCampaignSet.insert((*p).second->campaign_id);

            }
            if ((*p).second->Recommended != "")
            {
              if ((*p).second->brending)
              {
                  for(auto pr = result.begin(); pr != result.end(); ++pr)
                    {
                        if(OutPutOfferSet.count((*pr).second->id_int) == 0 && (*pr).second->is_recommended && (*p).second->campaign_id == (*pr).second->campaign_id )
                        {
                            if(vResult.size() >= params->getCapacity())
                                break;
                            
                            vResult.push_back((*pr).second);
                            OutPutOfferSet.insert((*pr).second->id_int);
                            OutPutCampaignSet.insert((*pr).second->campaign_id);

                        }
                    }
                }
            }
        }
        passage++;
    }

        //add teaser when teaser unique id
        for(auto p = result.begin(); p!=result.end(); ++p)
        {
            if(OutPutCampaignSet.count((*p).second->campaign_id) < (*p).second->unique_by_campaign
                    && OutPutOfferSet.count((*p).second->id_int) == 0 && !(*p).second->is_recommended)
            {
                if(vResult.size() >= params->getCapacity())
                    break;
                
                vResult.push_back((*p).second);
                OutPutOfferSet.insert((*p).second->id_int);

            }
            if ((*p).second->Recommended != "")
            {
              for(auto pr = result.begin(); pr != result.end(); ++pr)
                {
                    if(OutPutCampaignSet.count((*pr).second->campaign_id) < (*pr).second->unique_by_campaign
                            && OutPutOfferSet.count((*pr).second->id_int) == 0 && (*pr).second->is_recommended)
                    {
                        if(vResult.size() >= params->getCapacity())
                            break;
                        
                        vResult.push_back((*pr).second);
                        OutPutOfferSet.insert((*pr).second->id_int);

                    }
                }

            }
        }

    if(vResult.size() > params->getCapacity())
    {
        vResult.erase(vResult.begin() + params->getCapacity(), vResult.end());
    }
    //Offer load
    for(auto p = vResult.begin(); p != vResult.end(); ++p)
    {
        (*p)->load();
        (*p)->gen();
    }
    #ifdef DEBUG
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        printf("Time %s taken return RIS offers %lu: %lld \n", __func__,  vResult.size(), microseconds);
        printf("%s\n","------------------------------------------------------------------");
    #endif // DEBUG
}
