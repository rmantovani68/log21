-- select md5(random()::text || clock_timestamp()::text)::uuid;
select overlay(overlay(md5(random()::text || ':' || clock_timestamp()::text) placing '4' from 13) placing '8' from 17)::uuid;
