drop function frattomille(bigint);
CREATE FUNCTION frattomille(bigint) returns numeric(11,3) as 'select ($1::float/1000::float)::numeric(11,3)' language 'sql';
drop function frattomille(integer);
CREATE FUNCTION frattomille(integer) returns numeric(11,3) as 'select ($1::float/1000::float)::numeric(11,3)' language 'sql';
