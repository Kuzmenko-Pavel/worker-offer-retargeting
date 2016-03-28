#ifndef CORE_H
#define CORE_H

#include <set>

#include <boost/date_time.hpp>

#include "Offer.h"
#include "Params.h"
#include "Core_DataBase.h"
#include "json.h"


/// Класс, который связывает воедино все части системы.
class Core : public Core_DataBase
{
public:
    Core();
    ~Core();

    /** \brief  Обработка запроса на показ рекламы.
     * Самый главный метод. Возвращает HTML-строку, которую нужно вернуть
     * пользователю.
     */
    std::string Process(Params *params);
    std::string UserCode(Params *params);
    /** \brief save process results to mongodb log */
    void ProcessSaveResults();

private:
    boost::posix_time::ptime
    ///start and ent time core process
    startCoreTime, endCoreTime, endCampaignTime;
    ///core thread id
    pthread_t tid;
    ///parameters to process: from http GET
    Params *params;
    ///return string
    std::string retHtml;
    ///result offers vector
    Offer::Vector vResult;
    ///all offers to show
    Offer::Map items;
    ///campaigns to show set
    std::multiset<unsigned long long> OutPutCampaignSet;
    ///offers to show set
    std::set<unsigned long long> OutPutOfferSet;
    /** \brief Основной алгоритм отбора РП RealInvest Soft. */
    void RISAlgorithm(const Offer::Map &items);
    /** \brief  Возвращает json-представление предложений ``items`` */
    nlohmann::json OffersToJson(Offer::Vector &items);
    /** \brief  Возвращает безопасную json строку (экранирует недопустимые символы) */
    static std::string EscapeJson(const std::string &str);
    /** \brief logging Core result in syslog */
    void log();
    /** \brief make result html */
    void resultHtml();
};

#endif // CORE_H
