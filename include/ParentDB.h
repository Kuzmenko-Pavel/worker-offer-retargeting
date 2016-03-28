#ifndef PARENTDB_H
#define PARENTDB_H

#include <mongo/client/dbclient_rs.h>

#include "KompexSQLiteDatabase.h"
#include "KompexSQLiteException.h"

class ParentDB
{
public:
    ParentDB();
    virtual ~ParentDB();

    void OfferLoad(mongo::Query, mongo::BSONObj &camp);
    void OfferRemove(const std::string &id);
    void CampaignLoad(const std::string &campaignId = std::string());
    void CampaignLoad(mongo::Query=mongo::Query());
    void CampaignRemove(const std::string &aCampaignId);

private:
    bool fConnectedToMainDatabase;
    Kompex::SQLiteDatabase *pdb;
    char buf[262144];
    mongo::DBClientReplicaSet *monga_main;

    void logDb(const Kompex::SQLiteException &ex) const;


    bool ConnectMainDatabase();
};

#endif // PARENTDB_H
