#include "Core_DataBase.h"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <vector>
#include <map>
#include <chrono>
#include "KompexSQLiteStatement.h"
#include "KompexSQLiteException.h"
#include "Config.h"
#include "Offer.h"
#include "../config.h"

#define CMD_SIZE 2621440

Core_DataBase::Core_DataBase():
    len(CMD_SIZE)
{
    cmd = new char[len];
}

Core_DataBase::~Core_DataBase()
{
    delete []cmd;
}

//-------------------------------------------------------------------------------------------------------------------
bool Core_DataBase::getOffers(Offer::Map &items, Params *_params)
{
    bool result = false;
    #ifdef DEBUG
        auto start = std::chrono::high_resolution_clock::now();
        printf("%s\n","------------------------------------------------------------------");
    #endif // DEBUG
    Kompex::SQLiteStatement *pStmt;
    params = _params;
    std::string offerSqlStr;
    std::string select_field;
    std::string where_offers;
    std::string order_offers;
    std::string limit_offers;
    
    offerSqlStr = "\
        SELECT %s\
        FROM Offer AS ofrs\
        LEFT JOIN Offer2Rating AS oret ON ofrs.id=oret.id\
        LEFT JOIN Informer2OfferRating AS iret ON iret.id_inf=%lld AND ofrs.id=iret.id_ofr\
        %s %s %s ;\
       ";
    select_field ="\
        ofrs.id,\
        ofrs.campaignId,\
        ifnull(iret.rating,oret.rating) AS rating,\
        ofrs.UnicImpressionLot,\
        ofrs.social,\
        ofrs.offer_by_campaign_unique,\
        ofrs.html_notification\
        ";
    where_offers = "WHERE ofrs.campaignId IN ("+ params->getCampaigns() +") AND ofrs.id NOT IN ( SELECT [offerId] FROM Session where id=" + params->getUserKey() +" and uniqueHits <= 0)"; 
    if (!params->getExclude().empty())
    {
        where_offers = "WHERE ofrs.campaignId IN ("+ params->getCampaigns() +") AND ofrs.id NOT IN ("+ params->getExclude() +")"; 
    }
    order_offers = "ORDER BY rating DESC"; 
    limit_offers = "LIMIT " + std::to_string(params->getCapacity() * 5); 

    bzero(cmd,sizeof(cmd));
    sqlite3_snprintf(len, cmd, offerSqlStr.c_str(),
                     select_field.c_str(),
                     params->getInformerIdInt(),
                     where_offers.c_str(),
                     order_offers.c_str(),
                     limit_offers.c_str());
    try
    {
        pStmt = new Kompex::SQLiteStatement(cfg->pDb->pDatabase);

        try
        {
            pStmt->Sql(cmd);

            while(pStmt->FetchRow())
            {

                if(items.count(pStmt->GetColumnInt64(0)) > 0)
                {
                    continue;
                }

                Offer *off = new Offer(pStmt->GetColumnInt64(0),     //id
                                       pStmt->GetColumnInt64(1),    //campaignId
                                       pStmt->GetColumnDouble(2),   //rating
                                       pStmt->GetColumnInt(3),    //uniqueHits
                                       pStmt->GetColumnBool(4),    //social
                                       pStmt->GetColumnInt(5),      //offer_by_campaign_unique
                                       pStmt->GetColumnBool(6)    //notification
                                      );
                items.insert(Offer::Pair(off->id_int,off));
                result = true;
            }
        }
        catch(Kompex::SQLiteException &ex)
        {
            std::clog<<"["<<pthread_self()<<"]"<<__func__<<" error: "
                     <<ex.GetString()
                     <<" \n"
                     <<cmd
                     <<params->get_.c_str()
                     <<params->post_.c_str()
                     <<std::endl;
        }

        pStmt->FreeQuery();
        delete pStmt;

    }
    catch(std::exception const &ex)
    {
        std::clog<<"["<<pthread_self()<<"]"<<__func__<<" error: "
                 <<ex.what()
                 <<" \n"
                 <<cmd
                 <<std::endl;
    }
    where_offers = "WHERE ofrs.campaignId IN ("+ params->getSocialCampaigns() +") AND ofrs.id NOT IN ( SELECT [offerId] FROM Session where id=" + params->getUserKey() +" and uniqueHits <= 0)"; 
    if (!params->getExclude().empty())
    {
        where_offers = "WHERE ofrs.campaignId IN ("+ params->getSocialCampaigns() +") AND ofrs.id NOT IN ("+ params->getExclude() +")"; 
    }

    bzero(cmd,sizeof(cmd));
    sqlite3_snprintf(len, cmd, offerSqlStr.c_str(),
                     select_field.c_str(),
                     params->getInformerIdInt(),
                     where_offers.c_str(),
                     order_offers.c_str(),
                     limit_offers.c_str());
    try
    {
        pStmt = new Kompex::SQLiteStatement(cfg->pDb->pDatabase);

        try
        {
            pStmt->Sql(cmd);

            while(pStmt->FetchRow())
            {

                if(items.count(pStmt->GetColumnInt64(0)) > 0)
                {
                    continue;
                }

                Offer *off = new Offer(pStmt->GetColumnInt64(0),     //id
                                       pStmt->GetColumnInt64(1),    //campaignId
                                       pStmt->GetColumnDouble(2),   //rating
                                       pStmt->GetColumnInt(3),    //uniqueHits
                                       pStmt->GetColumnBool(4),    //social
                                       pStmt->GetColumnInt(5),      //offer_by_campaign_unique
                                       pStmt->GetColumnBool(6)    //notification
                                      );
                items.insert(Offer::Pair(off->id_int,off));
                result = true;
            }
        }
        catch(Kompex::SQLiteException &ex)
        {
            std::clog<<"["<<pthread_self()<<"]"<<__func__<<" error: "
                     <<ex.GetString()
                     <<" \n"
                     <<cmd
                     <<params->get_.c_str()
                     <<params->post_.c_str()
                     <<std::endl;
        }

        pStmt->FreeQuery();
        delete pStmt;

    }
    catch(std::exception const &ex)
    {
        std::clog<<"["<<pthread_self()<<"]"<<__func__<<" error: "
                 <<ex.what()
                 <<" \n"
                 <<cmd
                 <<std::endl;
    }
    //clear if there is(are) banner
    #ifdef DEBUG
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        printf("Time %s taken: %lld \n", __func__,  microseconds);
        printf("%s\n","------------------------------------------------------------------");
    #endif // DEBUG
    return result;
}
