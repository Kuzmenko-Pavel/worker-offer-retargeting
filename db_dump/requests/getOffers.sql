SELECT %s
FROM %s AS cn
INNER JOIN Offer AS ofrs ON ofrs.retargeting=%d AND ofrs.campaignId=cn.id  %s
LEFT JOIN Offer2Rating AS oret ON ofrs.id=oret.id
LEFT JOIN Informer2OfferRating AS iret ON iret.id_inf=%lld AND ofrs.id=iret.id_ofr
%s %s %s ;
