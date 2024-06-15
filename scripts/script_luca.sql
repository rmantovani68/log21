



create table mail_vettori (mvcdvet text, mvemail text);

create sequence registro_mail_sequence;
create table registro_mail 
(rmprogr int default nextval('registro_mail_sequence'),
rmstato text default ' ',
rmemail text default ' ',
rmogget text default ' ',
rmtesto text default ' ',
rmalleg text default ' ',
rmtminv timestamp default current_timestamp);
