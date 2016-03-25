CREATE TABLE IF NOT EXISTS Informer2OfferRating
(
id INTEGER PRIMARY KEY AUTOINCREMENT,
id_inf INT8 NOT NULL,
id_ofr INT8 NOT NULL,
rating DECIMAL(10,4),
UNIQUE (id_inf,id_ofr) ON CONFLICT IGNORE,
FOREIGN KEY(id_ofr) REFERENCES Offer(id) ON DELETE CASCADE
);
CREATE INDEX IF NOT EXISTS idx_Informer2OfferRating_id_inf ON Informer2OfferRating (id_inf DESC);
CREATE INDEX IF NOT EXISTS idx_Informer2OfferRating_id_ofr ON Informer2OfferRating (id_ofr DESC);
CREATE INDEX IF NOT EXISTS idx_Informer2OfferRating_id_inf_id_ofr ON Informer2OfferRating (id_inf DESC, id_ofr DESC);
CREATE INDEX IF NOT EXISTS idx_Informer2OfferRating_id_inf_id_ofr_rating ON Informer2OfferRating (id_ofr DESC, id_ofr DESC, rating DESC);
