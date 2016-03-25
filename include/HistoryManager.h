#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

#include <string>
#include <list>

#include "Offer.h"
#include "Params.h"
#include "Config.h"

class HistoryManager
{
public:
    /// Счётчик обработанных запросов
    static int request_processed_;
    static int offer_processed_;
    static int social_processed_;
    //Задаем значение очистки истории показов
    bool place_clean;
    bool retargeting_clean;
    bool retargeting_account_clean;
    bool place_get;
    bool place_find;
    bool retargeting_get;
    bool retargeting_account_get;
    bool retargeting_find;
    bool retargeting_account_find;
    bool social_get;
    bool social_find;

    HistoryManager();
    virtual ~HistoryManager();

    //main methods
    bool updateUserHistory(const Offer::Vector &outItems);
    void startGetUserHistory(Params *params);

protected:
private:
    std::string key;
    Params *params;

    bool setDeprecatedOffers(const Offer::Vector &items);
};
#endif
