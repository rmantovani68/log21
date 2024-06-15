-- 018155655  L' ANGOLO MANZONI SAS  da TO2 -> a BRU
-- 018155656  L' ANGOLO MANZONI SAS  da SGM -> a BRU
-- 018157613 , 018157610 , 018157615 da sgm a BRU

-- ordine 018204502: da AK1 a BRU
-- ordine 018204853: da SEG a SEU


select '|'||rocdlin||'|','|'||rodslin||'|' from ric_ord where ordprog in ('018289206'  );
select '|'||rocdlin||'|','|'||rodslin||'|' from ric_ord where rocdlin in ('BH4' ) limit 1;

-- |BRU|    | |BRT ORDINI URGENTI            |
-- |BRU|    | |BRT ORDINI URGENTI            |
-- |SEU|    | |NAVETTA SEGRATE ORDINI URGENTI|


-- select '|'||rocdlin||'|','|'||rodslin||'|' from ric_ord where rocdlin in ('BRU' ) limit 1;

-- update ric_ord set rocdlin='BRU', rodslin='BRT ORDINI URGENTI            ' where ordprog =  '018227780';
update ric_ord set rocdlin='BH4', rodslin='CARPIANO A PALLET             ' where ordprog =  '018289206';

select '|'||rocdlin||'|','|'||rodslin||'|' from ric_ord where ordprog in ('018289206' );
