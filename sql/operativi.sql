pg_dump mondadori -t settori     > backup_operativi.sql
pg_dump mondadori -t operatori  >> backup_operativi.sql
pg_dump mondadori -t displays   >> backup_operativi.sql
pg_dump mondadori -t ubicazioni >> backup_operativi.sql
pg_dump mondadori -t imballi    >> backup_operativi.sql
