\echo 'current-queries';
\echo '---------------';
SELECT procpid, age(query_start, clock_timestamp()), usename, current_query 
FROM pg_stat_activity 
WHERE current_query != '<IDLE>' AND current_query NOT ILIKE '%pg_stat_activity%' ORDER BY query_start desc;


\echo 'list-users';
\echo '---------------';
select * from pg_stat_activity where current_query not like '<%';

\echo 'list-tables';
\echo '---------------';
select datname, pg_size_pretty(pg_database_size(datname))
from pg_database
order by pg_database_size(datname) desc;

\echo 'cache hit rates (should not be less than 0.99)'
\echo '----------------------------------------------'
SELECT sum(heap_blks_read) as heap_read, sum(heap_blks_hit)  as heap_hit, (sum(heap_blks_hit) - sum(heap_blks_read)) / sum(heap_blks_hit) as ratio
FROM pg_statio_user_tables;

\echo 'table index usage rates (should not be less than 0.99)'
\echo '------------------------------------------------------'
SELECT relname, 100 * idx_scan / (seq_scan + idx_scan) percent_of_times_index_used, n_live_tup rows_in_table
FROM pg_stat_user_tables 
ORDER BY n_live_tup DESC;

\echo 'how many indexes are in cache'
\echo '-----------------------------'
SELECT sum(idx_blks_read) as idx_read, sum(idx_blks_hit)  as idx_hit, (sum(idx_blks_hit) - sum(idx_blks_read)) / sum(idx_blks_hit) as ratio
FROM pg_statio_user_indexes;
