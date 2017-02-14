#include <vector>
#include <boost/algorithm/string.hpp>
#include "../config.h"

#include "ParentDB.h"
#include "Log.h"
#include <mongocxx/instance.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/read_preference.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/options/find.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/types/value.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/document/view.hpp>
#include <KompexSQLiteStatement.h>
#include "json.h"
#include "Config.h"
#include "Offer.h"

using bsoncxx::builder::basic::document;
using bsoncxx::builder::basic::sub_document;
using bsoncxx::builder::basic::kvp;
using mongocxx::options::find;
using mongocxx::read_preference;

ParentDB::ParentDB()
{
    pdb = Config::Instance()->pDb->pDatabase;
}

ParentDB::~ParentDB()
{
}



void ParentDB::OfferLoad(document &query, nlohmann::json &camp)
{
    Kompex::SQLiteStatement *pStmt;
    unsigned int transCount = 0;
    unsigned int skipped = 0;
    
    pStmt = new Kompex::SQLiteStatement(pdb);
    nlohmann::json o = camp["showConditions"];
    std::string campaignId = camp["guid"].get<std::string>();
    mongocxx::client conn{mongocxx::uri{cfg->mongo_main_url_}};
    conn.read_preference(read_preference(read_preference::read_mode::k_secondary_preferred));
    auto coll = conn[cfg->mongo_main_db_]["offer"];
    auto cursor = coll.find(query.view());

    std::vector<std::string> items;
    try{
        pStmt->BeginTransaction();
        for (auto &&doc : cursor)
        {
               items.push_back(bsoncxx::to_json(doc));
        }
        for(auto i : items) {
            nlohmann::json x = nlohmann::json::parse(i);
            std::string id = x["guid"].get<std::string>();
            if (id.empty())
            {
                skipped++;
                continue;
            }

            std::string image = x["image"].get<std::string>();
            if (image.empty())
            {
                skipped++;
                continue;
            }

            bzero(buf,sizeof(buf));
            sqlite3_snprintf(sizeof(buf),buf,
                "INSERT OR REPLACE INTO Offer (\
                id,\
                guid,\
                retid,\
                campaignId,\
                image,\
                uniqueHits,\
                brending,\
                description,\
                url,\
                Recommended,\
                recomendet_type,\
                recomendet_count,\
                title,\
                campaign_guid,\
                social,\
                offer_by_campaign_unique,\
                account,\
                target,\
                UnicImpressionLot,\
                html_notification\
                )\
                VALUES(\
                        %llu,\
                        '%q',\
                        '%q',\
                        %llu,\
                        '%q',\
                        %d,\
                        %d,\
                        '%q',\
                        '%q',\
                        '%q',\
                        '%q',\
                        %d,\
                        '%q',\
                        '%q',\
                        %d,\
                        %d,\
                        '%q',\
                        '%q',\
                        %d,\
                        %d);",
                x["guid_int"].get<long long>(),
                id.c_str(),
                x["RetargetingID"].get<std::string>(),
                x["campaignId_int"].get<long long>(),
                image.c_str(),
                x["uniqueHits"].get<int>(),
                o["brending"].get<bool>() ? 1 : 0,
                x["description"].get<std::string>(),
                x["url"].get<std::string>(),
                x["Recommended"].get<std::string>(),
                 o["recomendet_type"].is_string() ? o["recomendet_type"].get<std::string>() : "all",
                 o["recomendet_count"].is_number() ? o["recomendet_count"].get<int>() : 10,
                x["title"].get<std::string>(),
                campaignId.c_str(),
                camp["social"].get<bool>() ? 1 : 0,
                o["offer_by_campaign_unique"].is_number() ? o["offer_by_campaign_unique"].get<int>() : 1,
                camp["account"].get<std::string>(),
                o["target"].get<std::string>(),
                o["UnicImpressionLot"].is_number() ? o["UnicImpressionLot"].get<int>() : 1,
                o["html_notification"].get<bool>() ? 1 : 0);
    
            try
            {
                pStmt->SqlStatement(buf);
            }
            catch(Kompex::SQLiteException &ex)
            {
                logDb(ex);
                skipped++;
            }
            transCount++;
            if (transCount % 1000 == 0)
            {
                pStmt->CommitTransaction();
                pStmt->FreeQuery();
                pStmt->BeginTransaction();
            }

        }
        pStmt->CommitTransaction();
        pStmt->FreeQuery();
    }
    catch(std::exception const &ex)
    {
        std::clog<<"["<<pthread_self()<<"]"<<__func__<<" error: "
                 <<ex.what()
                 <<" \n"
                 <<std::endl;
    }

    pStmt->FreeQuery();
    delete pStmt;

    Log::info("Loaded %d offers", transCount);
    if (skipped)
        Log::warn("Offers with empty id or image skipped: %d", skipped);
}
void ParentDB::OfferRemove(const std::string &id)
{
    Kompex::SQLiteStatement *pStmt;

    if(id.empty())
    {
        return;
    }

    pStmt = new Kompex::SQLiteStatement(pdb);
    sqlite3_snprintf(sizeof(buf),buf,"DELETE FROM Offer WHERE campaign_guid='%q';",id.c_str());
    try
    {
        pStmt->SqlStatement(buf);
    }
    catch(Kompex::SQLiteException &ex)
    {
        logDb(ex);
    }

    pStmt->FreeQuery();

    delete pStmt;

    Log::info("offer %s removed",id.c_str());
}
//-------------------------------------------------------------------------------------------------------


void ParentDB::logDb(const Kompex::SQLiteException &ex) const
{
    std::clog<<"ParentDB::logDb error: "<<ex.GetString()<<std::endl;
    std::clog<<"ParentDB::logDb request: "<<buf<<std::endl;
    #ifdef DEBUG
    printf("%s\n",ex.GetString().c_str());
    printf("%s\n",buf);
    #endif // DEBUG
}
/** \brief  Закгрузка всех рекламных кампаний из базы данных  Mongo

 */
//==================================================================================
void ParentDB::CampaignLoad(const std::string &aCampaignId)
{
    auto filter = document{};

    if(!aCampaignId.empty())
    {
        filter.append(
                     kvp("showConditions.retargeting", bsoncxx::types::b_bool{true}),
                      kvp("showConditions.retargeting_type", "offer"),
                      kvp("status", "working"),
                      kvp("guid", aCampaignId));
    }
    else
    {
        filter.append(
                     kvp("showConditions.retargeting", bsoncxx::types::b_bool{true}),
                      kvp("showConditions.retargeting_type", "offer"),
                      kvp("status", "working")
                      );
    }
    CampaignLoad(filter);
}
/** \brief  Закгрузка всех рекламных кампаний из базы данных  Mongo

 */
//==================================================================================
void ParentDB::CampaignLoad(document &query)
{
    int count = 0;
    mongocxx::client conn{mongocxx::uri{cfg->mongo_main_url_}};
    conn.read_preference(read_preference(read_preference::read_mode::k_secondary_preferred));
    auto coll = conn[cfg->mongo_main_db_]["campaign"];
    auto cursor = coll.find(query.view());

    std::vector<std::string> items;
    try{
        for (auto &&doc : cursor)
        {
               items.push_back(bsoncxx::to_json(doc));
        }
        for(auto i : items) {
            nlohmann::json x = nlohmann::json::parse(i);
            std::string id = x["guid"].get<std::string>();
            if (id.empty())
            {
                Log::warn("Campaign with empty id skipped");
                continue;
            }

            std::string status = x["status"].get<std::string>();
            
            CampaignRemove(id);

            if (status != "working")
            {
                Log::info("Campaign is hold: %s", id.c_str());
                continue;
            }

            //------------------------Create CAMP-----------------------
            //Загрузили все предложения
            auto filter = document{};
            count++;
            filter.append(kvp("campaignId", id ));
            OfferLoad(filter, x);
            Log::info("Loaded campaign: %s", id.c_str());
        }//endfor
    }
    catch(std::exception const &ex)
    {
        std::clog<<"["<<pthread_self()<<"]"<<__func__<<" error: "
                 <<ex.what()
                 <<" \n"
                 <<std::endl;
    }

    Log::info("Loaded %d campaigns",count); 
}


void ParentDB::CampaignRemove(const std::string &CampaignId)
{
    if(CampaignId.empty())
    {
        return;
    }

    Kompex::SQLiteStatement *pStmt;
    pStmt = new Kompex::SQLiteStatement(pdb);
    bzero(buf,sizeof(buf));
    sqlite3_snprintf(sizeof(buf),buf,"DELETE FROM Offer WHERE campaign_guid='%s';",CampaignId.c_str());
    try
    {
        pStmt->SqlStatement(buf);
    }
    catch(Kompex::SQLiteException &ex)
    {
        logDb(ex);
    }

    pStmt->FreeQuery();

    delete pStmt;
}
