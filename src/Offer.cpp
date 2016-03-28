#include <sstream>

#include "Offer.h"
#include "Log.h"
#include "KompexSQLiteStatement.h"
#include "KompexSQLiteException.h"
#include "Config.h"
#include "json.h"
#include "../config.h"

#define CMD_SIZE 2621440

Offer::Offer(unsigned long long id_int,
             unsigned long long campaign_id,
             float rating,
             int uniqueHits,
             bool social,
             std::string account_id,
             int unique_by_campaign,
             std::string Recommended,
             std::string retid,
             bool brending,
             bool is_recommended,
             bool html_notification
             ):
    id_int(id_int),
    campaign_id(campaign_id),
    len(CMD_SIZE),
    rating(rating),
    uniqueHits(uniqueHits),
    social(social),
    account_id(account_id),
    unique_by_campaign((unsigned)unique_by_campaign),
    Recommended(Recommended),
    retid(retid),
    brending(brending),
    is_recommended(is_recommended),
    html_notification(html_notification)
{
    cmd = new char[len];
}

Offer::~Offer()
{
    delete []cmd;
}

nlohmann::json Offer::toJson() const
{
    #ifdef DEBUG
        auto start = std::chrono::high_resolution_clock::now();
    #endif // DEBUG
    nlohmann::json j;
    j["id"] = id_int;
    j["guid"] = id;
    j["title"] = title;
    j["description"] = description;
    j["price"] = "";
    j["image"] = image_url;
    j["url"] = url;
    j["token"] = token;
    j["rating"] = rating;
    j["campaign_id"] = campaign_id;
    j["campaign_guid"] = campaign_guid;
    j["unique"] = uniqueHits;
    if (is_recommended)
    {
        j["c"] = "Rec";
        j["branch"] = "NL32";
    }
    else
    {
        j["c"] = "Ret";
        j["branch"] = "NL31";
    }
    j["retargeting"] = true;
    j["is_recommended"] = is_recommended;
    j["retargeting_type"] = "offer";
    j["html_notification"] = html_notification;
    #ifdef DEBUG
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        printf("Time %s taken: %lld \n", __func__,  microseconds);
    #endif // DEBUG
    return j;
}

void Offer::gen()
{
    if(token_int == 0)
    {
        token_int = (long long int)rand();
        ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    }
    token = std::to_string(token_int) + std::to_string(ms.count()) + std::to_string(id_int);
}

void Offer::load()
{
    #ifdef DEBUG
        auto start = std::chrono::high_resolution_clock::now();
    #endif // DEBUG
    if(!loaded)
    {
        loaded = true;
        Kompex::SQLiteStatement *pStmt;
        std::string offerSqlStr;
        bzero(cmd,sizeof(cmd));
        offerSqlStr = "\
        SELECT \
        ofrs.guid,\
        ofrs.title,\
        ofrs.description,\
        ofrs.url,\
        ofrs.image,\
        ofrs.campaign_guid\
        FROM Offer AS ofrs\
        where ofrs.id= %llu;\
       ";
        sqlite3_snprintf(CMD_SIZE, cmd, offerSqlStr.c_str(), id_int);
        pStmt = new Kompex::SQLiteStatement(cfg->pDb->pDatabase);
        try
        {
            pStmt->Sql(cmd);
            while(pStmt->FetchRow())
            {
                id = pStmt->GetColumnString(0);
                title = pStmt->GetColumnString(1);
                description = pStmt->GetColumnString(2);
                url = pStmt->GetColumnString(3);
                image_url = pStmt->GetColumnString(4);
                campaign_guid = pStmt->GetColumnString(5);
                break;
            }
        }
        catch(Kompex::SQLiteException &ex)
        {
            std::clog<<"["<<pthread_self()<<"]"<<__func__<<" error: "
                     <<ex.GetString()
                     <<" \n"
                     <<cmd
                     <<std::endl;
        }
        pStmt->FreeQuery();
        delete pStmt;

    }
    #ifdef DEBUG
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        printf("Time %s taken: %lld \n", __func__,  microseconds);
    #endif // DEBUG
}
