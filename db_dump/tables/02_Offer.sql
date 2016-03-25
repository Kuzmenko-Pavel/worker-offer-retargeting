CREATE TABLE IF NOT EXISTS Offer
(
id INT8 PRIMARY KEY,
guid VARCHAR(36),
campaignId INT8,
image VARCHAR(2048),
uniqueHits SMALLINT,
description VARCHAR(70),
url VARCHAR(2048),
title  VARCHAR(35),
campaign_guid VARCHAR(64),
social SMALLINT,
offer_by_campaign_unique SMALLINT,
UnicImpressionLot SMALLINT,
html_notification SMALLINT,
UNIQUE (id) ON CONFLICT IGNORE,
UNIQUE (guid) ON CONFLICT IGNORE
) WITHOUT ROWID;

CREATE UNIQUE INDEX IF NOT EXISTS idx_Offer_guid ON Offer (guid DESC);

CREATE INDEX IF NOT EXISTS idx_Offer_camp ON Offer (campaignId DESC);

CREATE INDEX IF NOT EXISTS idx_Offer_id_camp ON Offer (id DESC, campaignId DESC);
