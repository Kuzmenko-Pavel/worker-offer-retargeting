#ifndef CORE_DATABASE_H
#define CORE_DATABASE_H

#include <string>
#include <set>

#include "Params.h"
#include "Offer.h"
#include "DataBase.h"

class Core_DataBase
{
    public:
        Core_DataBase();
        virtual ~Core_DataBase();

        bool getOffers(Offer::Map &items,Params *params);

    protected:
    private:
        Params *params;
        char *cmd;
        size_t len;
};

#endif // CORE_DATABASE_H
