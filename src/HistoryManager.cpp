#include "HistoryManager.h"
#include "Config.h"
#include "Log.h"
#include "KompexSQLiteStatement.h"
#include "KompexSQLiteException.h"
#include <boost/algorithm/string.hpp>
#include <string>
#include "../config.h"

#define SPHINX_CAPACITY_COUNT 2

HistoryManager::HistoryManager()
{
}

HistoryManager::~HistoryManager()
{
}

void HistoryManager::startGetUserHistory(Params *_params)
{
    params = _params;
    key = params->getUserKey();
}

bool HistoryManager::updateUserHistory(
    const Offer::Vector &outItems)
{

    //обновление deprecated
    setDeprecatedOffers(outItems);

    return true;
}

bool HistoryManager::setDeprecatedOffers(const Offer::Vector &items)
{
    char buf[8192];
    Kompex::SQLiteStatement *pStmt;

    if(place_clean)
    {
        try
        {
            #ifdef DEBUG
                printf(" %s place_clean \n", __func__);
            #endif // DEBUG
            pStmt = new Kompex::SQLiteStatement(cfg->pDb->pDatabase);
            sqlite3_snprintf(sizeof(buf),buf,"DELETE FROM Session WHERE id=%llu;"
                             ,params->getUserKeyLong());
            pStmt->SqlStatement(buf);
            delete pStmt;
        }
        catch(Kompex::SQLiteException &ex)
        {
            std::clog<<"HistoryManager::setDeprecatedOffers delete session error: "<<ex.GetString()<<std::endl;
        }
    place_clean = true;
    }

    pStmt = new Kompex::SQLiteStatement(cfg->pDb->pDatabase);
    for(auto it = items.begin(); it != items.end(); ++it)
    {
        try
        {
            sqlite3_snprintf(sizeof(buf),buf,
"INSERT OR REPLACE INTO Session(id,id_ofr,uniqueHits) \
VALUES(%llu,%llu,ifnull((SELECT uniqueHits FROM Session WHERE id=%llu AND id_ofr=%llu),%u)-1);",
                    params->getUserKeyLong(), (*it)->id_int,
                    params->getUserKeyLong(), (*it)->id_int, (*it)->uniqueHits);

            pStmt->SqlStatement(buf);
        }
        catch(Kompex::SQLiteException &ex)
        {
            std::clog<<"HistoryManager::setDeprecatedOffers error: "<<ex.GetString()<<" request:"<<buf<<std::endl;
        }
    }
    delete pStmt;

    return true;
}
