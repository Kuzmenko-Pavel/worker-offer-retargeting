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
    Offer::Map itemsR;
    std::vector<std::string> VRecommended;
    std::vector<std::string> temp_recommended;
    std::string offerSqlStr;
    std::string select_field;
    std::string where_offers;
    std::string order_offers;
    std::string limit_offers;
    
    offerSqlStr = "\
        SELECT %s\
        FROM Offer AS ofrs\
        %s %s %s ;\
       ";
    
    select_field ="\
        ofrs.id,\
        ofrs.campaignId,\
        10000000.0,\
        ofrs.UnicImpressionLot,\
        ofrs.social,\
        ofrs.account, \
        ofrs.offer_by_campaign_unique,\
        ofrs.Recommended,\
        ofrs.retid,\
        ofrs.brending, \
        ofrs.html_notification,\
        ofrs.recomendet_count, \
        ofrs.recomendet_type \
        ";
    order_offers = ""; 
    limit_offers = "LIMIT " + std::to_string(params->getCapacity() * 5); 
    int count = 0;
    std::string recomendet_type = "all";
    int recomendet_count = 10;
    int day = 0;
    std::vector<std::string> ret = params->getRetargetingOffers();
    if(!ret.size())
    {
        return result;
    }
    std::map<const unsigned long,int> retargeting_offers_day = params->getRetargetingOffersDayMap();
    
    where_offers = " WHERE (ofrs.campaignId IN ("+ params->getCampaigns() +") AND ofrs.id NOT IN ("+ params->getExclude() +")) ";
    where_offers += " AND (";
    unsigned int ic = 1;
    for (auto i=ret.rbegin(); i != ret.rend() ; ++i)
    {
        std::vector<std::string> par;
        boost::split(par, *i, boost::is_any_of("~"));
        if (!par.empty() && par.size() >= 3)
        {
            where_offers += "(ofrs.account='"+ par[2] +"' AND ofrs.target='"+ par[1] +"' AND ofrs.retid='"+ par[0] +"')";
            if (ic++ < ret.size())
            {
                where_offers += " OR ";
            }
        }

    }
    where_offers += " ) ";

    bzero(cmd,sizeof(cmd));
    sqlite3_snprintf(len, cmd, offerSqlStr.c_str(),
                     select_field.c_str(),
                     where_offers.c_str(),
                     order_offers.c_str(),
                     limit_offers.c_str());
    try
    {

        pStmt = new Kompex::SQLiteStatement(cfg->pDb->pDatabase);

        pStmt->Sql(cmd);

        while(pStmt->FetchRow())
        {
            recomendet_type = "all";
            recomendet_count = 10;
            Offer *off = new Offer(pStmt->GetColumnInt64(0),     //id
                                   pStmt->GetColumnInt64(1),    //campaignId
                                   pStmt->GetColumnDouble(2) - count,  //rating
                                   pStmt->GetColumnInt(3),    //uniqueHits
                                   pStmt->GetColumnBool(4),    //social
                                   pStmt->GetColumnString(5),  //account_id
                                   pStmt->GetColumnInt(6),      //offer_by_campaign_unique
                                   pStmt->GetColumnString(7),   //recomendet
                                   pStmt->GetColumnString(8),   //retid
                                   pStmt->GetColumnBool(9),    //brending
                                   false,                       //isrecomendet
                                   pStmt->GetColumnBool(10)    //notification
                                  );
            itemsR.insert(Offer::Pair(off->id_int,off));
            std::string qr = "";
            if(off->Recommended != "")
            {
                day = 0;
                std::string::size_type sz;
                long oi = std::stol (pStmt->GetColumnString(8),&sz);
                std::map<const unsigned long, int>::iterator it= retargeting_offers_day.find(oi);
                if( it != retargeting_offers_day.end() )
                {
                    day = it->second;
                }
                recomendet_count = pStmt->GetColumnInt(11);
                recomendet_type = pStmt->GetColumnString(12);
                if ( recomendet_type == "min")
                {
                    if (recomendet_count - day > 1)
                    {
                        recomendet_count = recomendet_count - day;
                    }
                    else
                    {
                        recomendet_count = 1;
                    }
                }
                else if ( recomendet_type == "max")
                {
                    if (1 + day < recomendet_count)
                    {
                        recomendet_count = 1 + day;
                    }
                }
                else
                {
                    if (recomendet_count < 1)
                    {
                        recomendet_count = 1;
                    }
                }
                temp_recommended.clear();
                boost::split(temp_recommended, off->Recommended, boost::is_any_of(","));
                if (temp_recommended.begin()+recomendet_count < temp_recommended.end())
                {
                    temp_recommended.erase(temp_recommended.begin()+recomendet_count, temp_recommended.end());
                }
                qr = "(ofrs.account='" + off->account_id + "' AND ofrs.retid IN (" + boost::algorithm::join(temp_recommended, ", ") + " ))";
                VRecommended.push_back(qr);
            }
            result = true;
            count++;
        }

        pStmt->FreeQuery();

        delete pStmt;
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
    if (VRecommended.size() > 0)
    {
        try
        {   
            where_offers = "";
            order_offers = ""; 
            limit_offers = "LIMIT 100"; 
            where_offers = " WHERE (ofrs.campaignId IN ("+ params->getCampaigns() +") AND ofrs.id NOT IN ("+ params->getExclude() +")) AND ( " + boost::algorithm::join(VRecommended, " OR ") + ") ";
            bzero(cmd,sizeof(cmd));
            sqlite3_snprintf(len, cmd, offerSqlStr.c_str(),
                             select_field.c_str(),
                             where_offers.c_str(),
                             order_offers.c_str(),
                             limit_offers.c_str());

            pStmt = new Kompex::SQLiteStatement(cfg->pDb->pDatabase);

            pStmt->Sql(cmd);

            while(pStmt->FetchRow())
            {
                Offer *off = new Offer(pStmt->GetColumnInt64(0),     //id
                                       pStmt->GetColumnInt64(1),    //campaignId
                                       pStmt->GetColumnDouble(2) - count,  //rating
                                       pStmt->GetColumnInt(3),    //uniqueHits
                                       pStmt->GetColumnBool(4),    //social
                                       pStmt->GetColumnString(5),  //account_id
                                       pStmt->GetColumnInt(6),      //offer_by_campaign_unique
                                       "",                          //recomendet
                                       pStmt->GetColumnString(8),   //retid
                                       false,                       //brending
                                       true,                        //isrecomendet
                                       false                        //notification
                                      );
                itemsR.insert(Offer::Pair(off->id_int,off));
                count++;
            }

            pStmt->FreeQuery();

            delete pStmt;
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
    }
    
    for(auto i = itemsR.begin(); i != itemsR.end(); i++)
    {
        items.insert(Offer::Pair((*i).first,(*i).second));
    }
    
    itemsR.clear();


    //clear if there is(are) banner
    #ifdef DEBUG
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        printf("Time %s taken: %lld \n", __func__,  microseconds);
        printf("%s\n","------------------------------------------------------------------");
    #endif // DEBUG
    return result;
}
