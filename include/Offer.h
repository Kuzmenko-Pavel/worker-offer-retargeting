#ifndef OFFER_H
#define OFFER_H

#include <string>
#include <list>
#include <map>
#include <vector>
#include <chrono>
#include "json.h"
#include "EBranch.h"

typedef long long			sphinx_int64_t;
typedef unsigned long long	sphinx_uint64_t;

/** \brief  Класс описывает рекламное предложение (например, товар или новость). */
class Offer
{
public:
    typedef std::map <const unsigned long,Offer*> Map;
    typedef std::multimap <const float,Offer*, std::greater<float>> MapRate;
    typedef std::map <const unsigned long,Offer*>::iterator it;
    typedef std::map <const unsigned long,Offer*>::const_iterator cit;
    typedef std::pair<const float,Offer*> PairRate;
    typedef std::pair<const unsigned long,Offer*> Pair;
    typedef std::vector <Offer*> Vector;
    typedef std::vector <Offer*>::iterator itV;

    typedef enum { banner, teazer, unknown } Type;
    /// Структура для хранения информации о рекламном предложении.
    std::string id;             ///< ID предложения
    unsigned long long id_int;                 ///< ID предложения
    std::string title;          ///< Заголовок
    std::string description;    ///< Описание
    std::string url;            ///< URL перехода на предложение
    std::string image_url;      ///< URL картинки
    unsigned long long campaign_id;    ///< ID кампании, к которой относится предложение
    float rating;				///< рейтинг РП
    int uniqueHits;		///< максимальное количество показов одному пользователю
    bool social;
    std::string campaign_guid;
    unsigned unique_by_campaign;
    bool html_notification;                 
    std::string account_id;
    std::string Recommended;
    std::string retid;
    bool brending;
    bool is_recommended;

    bool loaded = false;
    long long int token_int = 0;
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    std::string token;          ///< Токен для проверки ссылки
    
    Offer(unsigned long long id_int,
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
          bool html_notification);

    virtual ~Offer();

    //bool operator==(const Offer &other) const { return this->id_int == other.id_int; }
    //bool operator<(const Offer &other) const { return rating < other.rating; }
    /*
        bool operator<(const Offer *x1, const Offer *x2)
        {
            return x1->rating > x2->rating;
        }
    */
    // Каждому элементу просмотра присваиваем уникальный токен
    void gen();
    void load();

    nlohmann::json toJson() const;
    private:
        char *cmd;
        size_t len;
};

#endif // OFFER_H

