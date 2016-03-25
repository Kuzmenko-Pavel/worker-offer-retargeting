#include "EBranch.h"
/*
EBranch::EBranch()
{
    //ctor
}

EBranch::~EBranch()
{
    //dtor
}
*/
/*
По умолчанию все РП имеют ветку NL0 и преобретают какуето другую только после выбора его по какойто логической ветке. Так сделано для того что если вдруг какойто РП попадает в лог минуя ветки, оно отображаеться в графе "Вероятно сработало чтото нетак"
Вот правильное соответствие логической ветки и её названию. Надо проверить и поправить

NL1 Банер по показам - Места размешения
NL2 Банер по показам - Поисковый запрос
NL3 Банер по показам - Контекст запрос
NL4 Банер по показам - История поискового запроса
NL5 Банер по показам - История контекстного запроса
NL6 Банер по показам - История долгосрочная
NL7 Банер по кликам - Поисковый запрос
NL8 Банер по кликам - Контекст запрос
NL9 Банер по кликам - История поискового запроса
NL10 Банер по кликам - История контекстного запроса
NL11 Банер по кликам - История долгосрочная

NL17 Тизер по кликам - Поисковый запрос
NL18 Тизер по кликам - Контекст запрос
NL19 Тизер по кликам - История поискового запроса
NL20 Тизер по кликам - История контекстного запроса
NL21 Тизер по кликам - История долгосрочная

NL29 Банер и Тизер по кликам - Тематики
NL30 Банер и Тизер по кликам - Места размешения
NL31 Банер и Тизер по кликам - Ретаргетинг
*/
const std::string EBranchL2String(EBranchL b)
{
    switch(b)
    {
        case EBranchL::L0: return "NL0";//default
        case EBranchL::L1: return "NL1";//Банер по показам - Места размешения
        case EBranchL::L2: return "NL2";//Банер по показам - Поисковый запрос
        case EBranchL::L3: return "NL3";//Банер по показам - Контекст запрос
        case EBranchL::L4: return "NL4";//Банер по показам - История поискового запроса
        case EBranchL::L5: return "NL5";//Банер по показам - История контекстного запроса
        case EBranchL::L6: return "NL6";//Банер по показам - История долгосрочная
        case EBranchL::L7: return "NL7";//Банер по кликам - Поисковый запрос
        case EBranchL::L8: return "NL8";//Банер по кликам - Контекст запрос
        case EBranchL::L9: return "NL9";//Банер по кликам - История поискового запроса
        case EBranchL::L10: return "NL10";//Банер по кликам - История контекстного запроса
        case EBranchL::L11: return "NL11";//Банер по кликам - История долгосрочная
        case EBranchL::L12: return "NL12";
        case EBranchL::L13: return "NL13";
        case EBranchL::L14: return "NL14";
        case EBranchL::L15: return "NL15";
        case EBranchL::L16: return "NL16";
        case EBranchL::L17: return "NL17";//Тизер по кликам - Поисковый запрос
        case EBranchL::L18: return "NL18";//Тизер по кликам - Контекст запрос
        case EBranchL::L19: return "NL19";//Тизер по кликам - История поискового запроса
        case EBranchL::L20: return "NL20";//Тизер по кликам - История контекстного запроса
        case EBranchL::L21: return "NL21";//Тизер по кликам - История долгосрочная
        case EBranchL::L22: return "NL22";
        case EBranchL::L23: return "NL23";
        case EBranchL::L24: return "NL24";
        case EBranchL::L25: return "NL25";
        case EBranchL::L26: return "NL26";
        case EBranchL::L27: return "NL27";
        case EBranchL::L28: return "NL28";
        case EBranchL::L29: return "NL29";//Банер и Тизер по кликам - Тематики
        case EBranchL::L30: return "NL30";//Банер и Тизер по кликам - Места размешения
        case EBranchL::L31: return "NL31";//retargeting ris 1(teaser when teaser unique id and with company unique)
        case EBranchL::L32: return "NL32";//retargeting ris 2(teaser when teaser unique id and company <= unique_by_campaign)
        case EBranchL::L33: return "NL33";//tail
        case EBranchL::L34: return "NL34";//ris: banner select if not was changed before
        case EBranchL::L35: return "NL35";//ris: teaser by unique id and company if not was changed before
        case EBranchL::L36: return "NL36";//ris: teaser by unique id if not was changed before
        case EBranchL::L37: return "NL37";//ris: expand to return size if not was changed before
        default: return "NOP";
    }
};
