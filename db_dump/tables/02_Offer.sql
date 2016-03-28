CREATE TABLE IF NOT EXISTS Offer
(
id INT8 PRIMARY KEY,
guid VARCHAR(36),
retid VARCHAR(36) DEFAULT "",
campaignId INT8,
image VARCHAR(2048),
uniqueHits SMALLINT,
brending SMALLINT,
description VARCHAR(70),
url VARCHAR(2048),
Recommended VARCHAR(2048),
recomendet_type VARCHAR(3),
recomendet_count SMALLINT,
title  VARCHAR(35),
campaign_guid VARCHAR(64),
social SMALLINT,
offer_by_campaign_unique SMALLINT,
account VARCHAR(64),
target VARCHAR(100),
UnicImpressionLot SMALLINT,
html_notification SMALLINT,
UNIQUE (id) ON CONFLICT IGNORE,
UNIQUE (guid) ON CONFLICT IGNORE
) WITHOUT ROWID;

CREATE UNIQUE INDEX IF NOT EXISTS idx_Offer_guid ON Offer (guid DESC);
CREATE INDEX IF NOT EXISTS idx_Offer_camp ON Offer (campaignId DESC);
CREATE INDEX IF NOT EXISTS idx_Offer_id_camp ON Offer (id DESC, campaignId DESC);
CREATE INDEX IF NOT EXISTS idx_OfferR_retid ON Offer (retid DESC);
