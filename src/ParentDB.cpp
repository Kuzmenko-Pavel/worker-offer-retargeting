#include <vector>
#include <boost/algorithm/string.hpp>
#include "../config.h"

#include "ParentDB.h"
#include "Log.h"
#include "KompexSQLiteStatement.h"
#include "json.h"
#include "Config.h"
#include "Offer.h"

ParentDB::ParentDB()
{
    pdb = Config::Instance()->pDb->pDatabase;
    fConnectedToMainDatabase = false;
    ConnectMainDatabase();
}

ParentDB::~ParentDB()
{
    //dtor
}


bool ParentDB::ConnectMainDatabase()
{
    if(fConnectedToMainDatabase)
        return true;

    std::vector<mongo::HostAndPort> hvec;
    for(auto h = cfg->mongo_main_host_.begin(); h != cfg->mongo_main_host_.end(); ++h)
    {
        hvec.push_back(mongo::HostAndPort(*h));
        std::clog<<"Connecting to: "<<(*h)<<std::endl;
    }

    try
    {
        if(!cfg->mongo_main_set_.empty())
        {
            monga_main = new mongo::DBClientReplicaSet(cfg->mongo_main_set_, hvec);
            monga_main->connect();
        }


        if(!cfg->mongo_main_login_.empty())
        {
            std::string err;
            if(!monga_main->auth(cfg->mongo_main_db_,cfg->mongo_main_login_,cfg->mongo_main_passwd_, err))
            {
                std::clog<<"auth db: "<<cfg->mongo_main_db_<<" login: "<<cfg->mongo_main_login_<<" error: "<<err<<std::endl;
            }
            else
            {
                fConnectedToMainDatabase = true;
            }
        }
        else
        {
            fConnectedToMainDatabase = true;
        }
    }
    catch (mongo::UserException &ex)
    {
        std::clog<<"ParentDB::"<<__func__<<" mongo error: "<<ex.what()<<std::endl;
        return false;
    }

    return true;
}

void ParentDB::OfferLoad(mongo::Query q_correct, mongo::BSONObj &camp)
{
    if(!fConnectedToMainDatabase)
        return;
    Kompex::SQLiteStatement *pStmt;
    int i = 0,
    skipped = 0;
    
    pStmt = new Kompex::SQLiteStatement(pdb);
    mongo::BSONObj o = camp.getObjectField("showConditions");
    mongo::BSONObj f = BSON("guid"<<1<<"image"<<1<<"swf"<<1<<"guid_int"<<1<<"RetargetingID"<<1<<"campaignId_int"<<1<<"campaignId"<<1<<"campaignTitle"<<1
            <<"image"<<1<<"uniqueHits"<<1<<"description"<<1
            <<"url"<<1<<"Recommended"<<1<<"title"<<1);
    std::vector<mongo::BSONObj> bsonobjects;
    std::vector<mongo::BSONObj>::const_iterator x;
    std::string campaignId = camp.getStringField("guid");
    auto cursor = monga_main->query(cfg->mongo_main_db_ + ".offer", q_correct, 0, 0, &f);
    try{
        unsigned int transCount = 0;
        pStmt->BeginTransaction();
        while (cursor->more())
        {
            mongo::BSONObj itv = cursor->next();
            bsonobjects.push_back(itv.copy());
        }
        x = bsonobjects.begin();
        while(x != bsonobjects.end()) {
            std::string id = (*x).getStringField("guid");
            if (id.empty())
            {
                skipped++;
                continue;
            }

            std::string image = (*x).getStringField("image");
            if (image.empty())
            {
                skipped++;
                x++;
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
                (*x).getField("guid_int").numberLong(),
                id.c_str(),
                (*x).getStringField("RetargetingID"),
                (*x).getField("campaignId_int").numberLong(),
                (*x).getStringField("image"),
                (*x).getIntField("uniqueHits"),
                o.getBoolField("brending") ? 1 : 0,
                (*x).getStringField("description"),
                (*x).getStringField("url"),
                (*x).getStringField("Recommended"),
                 o.hasField("recomendet_type") ? o.getStringField("recomendet_type") : "all",
                 o.hasField("recomendet_count") ? o.getIntField("recomendet_count") : 10,
                (*x).getStringField("title"),
                campaignId.c_str(),
                camp.getBoolField("social") ? 1 : 0,
                o.hasField("offer_by_campaign_unique") ? o.getIntField("offer_by_campaign_unique") : 1,
                camp.getStringField("account"),
                o.getStringField("target"),
                o.hasField("UnicImpressionLot") ? o.getIntField("UnicImpressionLot") : 1,
                o.getBoolField("html_notification") ? 1 : 0);
    
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
            i++;
            x++;
            if (transCount % 1000 == 0)
            {
                pStmt->CommitTransaction();
                pStmt->FreeQuery();
                pStmt->BeginTransaction();
            }

        }
        pStmt->CommitTransaction();
        pStmt->FreeQuery();
        bsonobjects.clear();
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

    Log::info("Loaded %d offers", i);
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
    mongo::Query query;

    if(!aCampaignId.empty())
    {
        query = mongo::Query("{\"guid\":\""+ aCampaignId +"\", \"status\" : \"working\",\"showConditions.retargeting\":true}");
    }
    else
    {
        query = mongo::Query("{\"status\" : \"working\",\"showConditions.retargeting\":true}");
    }
    CampaignLoad(query);
}
/** \brief  Закгрузка всех рекламных кампаний из базы данных  Mongo

 */
//==================================================================================
void ParentDB::CampaignLoad(mongo::Query q_correct)
{
    std::unique_ptr<mongo::DBClientCursor> cursor;
    int i = 0;

    cursor = monga_main->query(cfg->mongo_main_db_ +".campaign", q_correct);
    try{
    while (cursor->more())
    {
        mongo::BSONObj x = cursor->next();
        std::string id = x.getStringField("guid");
        if (id.empty())
        {
            Log::warn("Campaign with empty id skipped");
            continue;
        }

        std::string status = x.getStringField("status");
        
        CampaignRemove(id);

        if (status != "working")
        {
            Log::info("Campaign is hold: %s", id.c_str());
            continue;
        }

        //------------------------Create CAMP-----------------------
        //Загрузили все предложения
        mongo::Query q;
        q = mongo::Query("{\"campaignId\" : \""+ id + "\"}");
        OfferLoad(q, x);
        Log::info("Loaded campaign: %s", id.c_str());
        i++;

    }//while
    }
    catch(std::exception const &ex)
    {
        std::clog<<"["<<pthread_self()<<"]"<<__func__<<" error: "
                 <<ex.what()
                 <<" \n"
                 <<std::endl;
    }

    Log::info("Loaded %d campaigns",i); 
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
    sqlite3_snprintf(sizeof(buf),buf,"DELETE FROM Offer WHERE guid='%s';",CampaignId.c_str());
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
