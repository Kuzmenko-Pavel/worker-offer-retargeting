#include "DB.h"

#include <boost/regex.hpp>

#include <sstream>

#include <mongo/util/net/hostandport.h>

#include "../config.h"

#include "Log.h"
#include "BaseCore.h"
#include "base64.h"
#include "Offer.h"
#include "Config.h"
#include "CpuStat.h"

#define MAXCOUNT 1000

BaseCore::BaseCore()
{
    time_service_started_ = boost::posix_time::second_clock::local_time();

    pdb = new ParentDB();

    LoadAllEntities();

    InitMessageQueue();
}

BaseCore::~BaseCore()
{
    delete pdb;
    delete amqp_;
}

std::string BaseCore::toString(AMQPMessage *m)
{
    unsigned len;
    char *pMes;

#ifdef AMQPCPP_OLD
    pMes = m->getMessage();
    len = strlen(pMes);
#else
    pMes = m->getMessage(&len);
#endif // AMQPCPP_OLD

    return std::string(pMes,len);
}

bool BaseCore::ProcessMQ()
{
    AMQPMessage *m;
    int stopCount;

    time_mq_check_ = boost::posix_time::second_clock::local_time();

    try
    {
        {
            // Проверка сообщений campaign.#
            mq_campaign_->Get(AMQP_NOACK);
            m = mq_campaign_->getMessage();
            stopCount = MAXCOUNT;
            while(m->getMessageCount() > -1 && stopCount--)
            {
                mq_log_.push_back(m->getRoutingKey() + ":" +toString(m) + "</br>");
                if(cfg->logMQ)
                {
                    std::clog<<"mq: cmd:"<<m->getRoutingKey()<<toString(m)<<std::endl;
                }

                if(m->getRoutingKey() == "campaign.update")
                {
                    std::string CampaignId = toString(m);
                    pdb->CampaignLoad(CampaignId);
                }
                else if(m->getRoutingKey() == "campaign.delete")
                {
                    pdb->CampaignRemove(toString(m));
                }
                else if(m->getRoutingKey() == "campaign.start")
                {
                    std::string CampaignId = toString(m);
                    pdb->CampaignLoad(CampaignId);
                }
                else if(m->getRoutingKey() == "campaign.stop")
                {
                    std::string CampaignId = toString(m);
                    pdb->CampaignRemove(CampaignId);
                }

                mq_campaign_->Get(AMQP_NOACK);
                m = mq_campaign_->getMessage();
            }
        }
    }
    catch (AMQPException &ex)
    {
        std::clog<<"AMQPException: "<<ex.getMessage()<<std::endl;
        mq_log_.push_back("error: "+ ex.getMessage());
    }
    return false;
}


/*
*  Загружает из основной базы данных следующие сущности:
*
*  - рекламные предложения;
*  - рекламные кампании;
*  - информеры.
*
*  Если в кампании нет рекламных предложений, она будет пропущена.
*/
void BaseCore::LoadAllEntities()
{
    if(Config::Instance()->pDb->reopen)
    {
        return;
    }
    //Загрузили все кампании
    pdb->CampaignLoad(std::string());

    cfg->pDb->indexRebuild();

}
/** \brief  Инициализация очереди сообщений (AMQP).

    Если во время инициализации произошла какая-либо ошибка, то сервис
    продолжит работу, но возможность оповещения об изменениях и горячего
    обновления будет отключена.
*/
void BaseCore::InitMessageQueue()
{
    try
    {
        // Объявляем точку обмена
        amqp_ = new AMQP(Config::Instance()->mq_path_);
        exchange_ = amqp_->createExchange();
        exchange_->Declare("getmyad", "topic", AMQP_AUTODELETE);

        // Составляем уникальные имена для очередей
        boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
        std::string postfix = to_iso_string(now);
        boost::replace_first(postfix, ".", ",");

        std::string mq_campaign_name( "getmyad.campaign." + postfix );

        // Объявляем очереди
        mq_campaign_ = amqp_->createQueue();
        mq_campaign_->Declare(mq_campaign_name, AMQP_AUTODELETE | AMQP_EXCLUSIVE);

        // Привязываем очереди
        exchange_->Bind(mq_campaign_name, "campaign.#");

        std::clog<<"Created ampq queues: "<<mq_campaign_name<<std::endl;
    }
    catch (AMQPException &ex)
    {
        std::clog<<"Error in AMPQ init: "<<ex.getMessage()<<std::endl;
    }
}

/** Возвращает расширенные данные о состоянии службы
 */
std::string BaseCore::Status(const std::string &server_name)
{
    std::stringstream out;

    // Время последнего обращения к статистике
    static boost::posix_time::ptime last_time_accessed;

    boost::posix_time::time_duration d;

    // Вычисляем количество запросов в секунду
    if (last_time_accessed.is_not_a_date_time())
        last_time_accessed = time_service_started_;
    boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
    int millisecs_since_last_access =
        (now - last_time_accessed).total_milliseconds();
    int millisecs_since_start =
        (now - time_service_started_).total_milliseconds();
    int requests_per_second_current = 0;
    int requests_per_second_average = 0;

    if (millisecs_since_last_access)
        requests_per_second_current =
            (request_processed_ - last_time_request_processed) * 1000 /
            millisecs_since_last_access;
    if (millisecs_since_start)
        requests_per_second_average = request_processed_ * 1000 /
                                      millisecs_since_start;

    last_time_accessed = now;
    last_time_request_processed = request_processed_;

    out << "<html>";
    out << "<head>";
    out << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/>";
    out << "<style type=\"text/css\">";
    out << "body { font-family: Arial, Helvetica, sans-serif; }";
    out << "h1, h2, h3 {font-family: \"georgia\", serif; font-weight: 400;}";
    out << "table { border-collapse: collapse; border: 1px solid gray; }";
    out << "td { border: 1px dotted gray; padding: 5px; font-size: 10pt; }";
    out << "th {border: 1px solid gray; padding: 8px; font-size: 10pt; }";
    out << "</style>";
    out << "</head>";
    out << "<body>";
#ifdef DUMMY
    out << "<h1>Состояние службы Yottos GetMyAd dummy worker</h1>";
#else
    out << "<h1>Состояние службы Yottos GetMyAd worker</h1>";
#endif
    out << "<table>";
    out << "<tr>";
    out << "<td>Обработано запросов:</td><td><b>" << request_processed_;
    out << "</b> (" << requests_per_second_current << "/сек, ";
    out << " в среднем " << requests_per_second_average << "/сек) ";
    out << "</td></tr>";
    out << "<tr><td>Общее кол-во показов:</td><td><b>" << offer_processed_;
    out << "</b> (" << social_processed_ << " из них социальная реклама) "<< "</td></tr>";
    out << "<tr><td>Общее кол-во ретаргетинговых показов:</td><td><b>" << retargeting_processed_<< "</b></td></tr>";
    out << "<tr><td>Имя сервера: </td> <td>" << (server_name.empty() ? server_name : "неизвестно") <<"</td></tr>";
    out << "<tr><td>IP сервера: </td> <td>" <<Config::Instance()->server_ip_ <<"</td></tr>";
    out << "<tr><td>Текущее время: </td> <td>"<<boost::posix_time::second_clock::local_time()<<"</td></tr>";
    out << "<tr><td>Время запуска:</td> <td>" << time_service_started_ <<"</td></tr>";
    out << "<tr><td>Количество ниток: </td> <td>" << Config::Instance()->server_children_<< "</td></tr>";
    out << "<tr><td>CPU user: </td> <td>" << CpuStat::cpu_user << "</td></tr>";
    out << "<tr><td>CPU sys: </td> <td>" << CpuStat::cpu_sys << "</td></tr>";
    out << "<tr><td>RAM: </td> <td>" << CpuStat::rss << "</td></tr>";

    out << "<tr><td>Основная база данных:</td> <td>" <<
        cfg->mongo_main_db_<< "/";
    out << "<br/>slave_ok = " << (cfg->mongo_main_slave_ok_? "true" : "false");
    out << "<br/>replica set=";
    if (cfg->mongo_main_set_.empty())
        out << "no set";
    else
        out << cfg->mongo_main_set_;
    out << "</td></tr>";
    out << "<tr><td>AMQP:</td><td>" << (amqp_? "активен" : "не активен") << "</td></tr>";
    out <<  "<tr><td>Сборка: </td><td>" << __DATE__ << " " << __TIME__<<"</td></tr>";
    out << "</table>";
    // Журнал сообщений AMQP
    out << "<p>Журнал AMQP: </p>"
        "<table>";
    std::string mq_log_s;
    for (auto it = mq_log_.begin(); it != mq_log_.end(); it++)
    {
        mq_log_s += *it;
    }
    out << "<tr><td>Последнее сообщение:</td><td>"<< mq_log_s<< "</td></tr>";
    out << "<tr><td>Последняя проверка сообщений:</td><td>"<< time_mq_check_ <<"</td><tr>"
        "</table>";

    out << "</body>";
    out << "</html>";

    return out.str();
}

bool BaseCore::cmdParser(const std::string &cmd, std::string &offerId, std::string &campaignId)
{
    boost::regex exp("Offer:(.*),Campaign:(.*)");
    boost::cmatch pMatch;

    if(boost::regex_match(cmd.c_str(), pMatch, exp))
    {
        offerId = pMatch[1];
        campaignId = pMatch[2];
        return true;
    }
    return false;
}

