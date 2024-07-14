--
-- PostgreSQL database dump
--

-- Dumped from database version 15.7 (Ubuntu 15.7-0ubuntu0.23.10.1)
-- Dumped by pg_dump version 15.7 (Ubuntu 15.7-0ubuntu0.23.10.1)

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: ubicazioni; Type: TABLE; Schema: public; Owner: roberto
--

CREATE TABLE public.ubicazioni (
    ubicazione text,
    codprod text,
    isola integer DEFAULT 0,
    settore integer DEFAULT 0,
    display integer DEFAULT 0,
    ios integer DEFAULT 0,
    cpu integer DEFAULT 0,
    modulo integer DEFAULT 0,
    riga integer DEFAULT 0,
    colonna integer DEFAULT 0,
    priorita integer DEFAULT 0,
    ubqtcas integer DEFAULT 0,
    ubqtmax integer DEFAULT 0,
    ubqtimp integer DEFAULT 0,
    ubqtpre integer DEFAULT 0,
    ubcdflg text,
    ubitipo text,
    cnistato text,
    insert_user text,
    insert_time timestamp without time zone,
    update_user text,
    update_time timestamp without time zone
);


ALTER TABLE public.ubicazioni OWNER TO roberto;

--
-- Data for Name: ubicazioni; Type: TABLE DATA; Schema: public; Owner: roberto
--

INSERT INTO public.ubicazioni VALUES ('A    0  2  1', '9788804765288 ', 0, 1, 1, 0, 0, 0, 2, 6, 2, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.81294');
INSERT INTO public.ubicazioni VALUES ('X    0 11  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1011, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  1  4', '', 0, 10, 10, 0, 0, 0, 18, 29, 109, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0 36  1', '', 0, 9, 9, 0, 0, 0, 21, 21, 108, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 37  1', '', 0, 12, 12, 0, 0, 0, 28, 29, 193, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 15  2', '', 0, 10, 10, 0, 0, 0, 30, 31, 167, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  4  2', '', 0, 10, 10, 0, 0, 0, 20, 27, 123, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 40  1', '', 0, 12, 12, 0, 0, 0, 29, 26, 196, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 13  2', '', 0, 10, 10, 0, 0, 0, 29, 31, 159, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 21  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1021, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  2  3', '', 0, 10, 10, 0, 0, 0, 19, 26, 114, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 22  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1022, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 43  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1043, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 34  1', '', 0, 12, 12, 0, 0, 0, 28, 28, 190, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 32  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1032, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 42  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1042, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  2  1', '', 0, 10, 10, 0, 0, 0, 19, 28, 116, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 16  4', '', 0, 10, 10, 0, 0, 0, 31, 25, 169, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  8  1', '', 0, 10, 10, 0, 0, 0, 23, 32, 140, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 73  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1073, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 44  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1044, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 83  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1083, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 57  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1057, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 66  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1066, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 55  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1055, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 74  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1074, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 56  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1056, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 89  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1089, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 98  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1098, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 43  1', '', 0, 12, 12, 0, 0, 0, 25, 25, 199, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0 27  1', '9791254720509 ', 0, 9, 9, 0, 0, 0, 20, 19, 99, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('B    0 36  1', '9788804776116 ', 0, 6, 6, 0, 0, 0, 15, 12, 72, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.904644');
INSERT INTO public.ubicazioni VALUES ('B    0 19  1', '9791221205282 ', 0, 5, 5, 0, 0, 0, 13, 9, 55, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.004721');
INSERT INTO public.ubicazioni VALUES ('X    0 45  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1045, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 94  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1094, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 19  1', '', 0, 10, 10, 0, 0, 0, 26, 27, 175, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  6  1', '', 0, 10, 10, 0, 0, 0, 17, 32, 132, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  5  1', '', 0, 10, 10, 0, 0, 0, 17, 28, 128, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0 12  1', '9788804778967 ', 0, 7, 7, 0, 0, 0, 18, 22, 84, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.963105');
INSERT INTO public.ubicazioni VALUES ('X    0 46  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1046, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  3  2', '', 0, 10, 10, 0, 0, 0, 19, 31, 119, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  3  3', '', 0, 10, 10, 0, 0, 0, 19, 30, 118, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 17  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1017, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 41  1', '', 0, 12, 12, 0, 0, 0, 29, 25, 197, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 52  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1052, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 36  1', '', 0, 12, 12, 0, 0, 0, 28, 30, 192, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 41  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1041, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 77  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1077, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 90  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1090, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 44  1', '', 0, 12, 12, 0, 0, 0, 25, 26, 200, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 18  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1018, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 11  4', '', 0, 10, 10, 0, 0, 0, 22, 29, 149, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 15  1', '', 0, 10, 10, 0, 0, 0, 30, 32, 168, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 10  1', '', 0, 10, 10, 0, 0, 0, 22, 28, 148, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 79  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1079, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0 30  1', '9791254720837 ', 0, 9, 9, 0, 0, 0, 20, 24, 102, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('A    0 19  1', '9788891836519 ', 0, 2, 2, 0, 0, 0, 5, 1, 19, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.079713');
INSERT INTO public.ubicazioni VALUES ('A    0  4  1', '9788891837301 ', 0, 1, 1, 0, 0, 0, 1, 4, 4, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.788029');
INSERT INTO public.ubicazioni VALUES ('X    0 12  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1012, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0 22  1', '9791281089587 ', 0, 8, 8, 0, 0, 0, 19, 24, 94, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.804726');
INSERT INTO public.ubicazioni VALUES ('X    0 10  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1010, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 20  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1020, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('B    0  7  1', '9788817163897 ', 0, 4, 4, 0, 0, 0, 11, 13, 43, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.338229');
INSERT INTO public.ubicazioni VALUES ('A    0  1  1', '9791221205305 ', 0, 1, 1, 0, 0, 0, 2, 7, 1, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.03813');
INSERT INTO public.ubicazioni VALUES ('D    0  4  1', '', 0, 10, 10, 0, 0, 0, 20, 28, 124, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('A    0 29  1', '9788856674026 ', 0, 3, 3, 0, 0, 0, 6, 3, 29, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.821401');
INSERT INTO public.ubicazioni VALUES ('X    0 67  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1067, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 13  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1013, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 24  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1024, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('A    0 32  1', '9791221206562 ', 0, 3, 3, 0, 0, 0, 6, 8, 32, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.738063');
INSERT INTO public.ubicazioni VALUES ('X    0 76  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1076, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  8  3', '', 0, 10, 10, 0, 0, 0, 23, 30, 138, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 91  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1091, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 11  1', '', 0, 10, 10, 0, 0, 0, 22, 32, 152, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  7  4', '', 0, 10, 10, 0, 0, 0, 18, 25, 133, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0  3  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1003, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.7714');
INSERT INTO public.ubicazioni VALUES ('D    0  2  4', '', 0, 10, 10, 0, 0, 0, 19, 25, 113, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0  6  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1006, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0  2  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1002, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.722136');
INSERT INTO public.ubicazioni VALUES ('C    0 19  1', '9788817180139 ', 0, 8, 8, 0, 0, 0, 19, 19, 91, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.288223');
INSERT INTO public.ubicazioni VALUES ('X    0 31  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1031, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('B    0 14  1', '9788806256425 ', 0, 5, 5, 0, 0, 0, 12, 14, 50, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.946409');
INSERT INTO public.ubicazioni VALUES ('X    0 65  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1065, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('B    0 13  1', '9788836281152 ', 0, 5, 5, 0, 0, 0, 12, 11, 49, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.321482');
INSERT INTO public.ubicazioni VALUES ('X    0 95  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1095, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 40  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1040, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('B    0 26  1', '9788891589347 ', 0, 6, 6, 0, 0, 0, 14, 10, 62, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.229762');
INSERT INTO public.ubicazioni VALUES ('C    0  3  1', '9791221206401 ', 0, 7, 7, 0, 0, 0, 17, 19, 75, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.188195');
INSERT INTO public.ubicazioni VALUES ('D    0 42  1', '', 0, 12, 12, 0, 0, 0, 29, 28, 198, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 26  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1026, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0 11  1', '9788866408192 ', 0, 7, 7, 0, 0, 0, 18, 19, 83, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.954666');
INSERT INTO public.ubicazioni VALUES ('B    0  6  1', '9788820075637 ', 0, 4, 4, 0, 0, 0, 11, 14, 42, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.313226');
INSERT INTO public.ubicazioni VALUES ('D    0 14  2', '', 0, 10, 10, 0, 0, 0, 30, 27, 163, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 36  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1036, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 71  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1071, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0  9  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1009, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0 21  1', '9788817181310 ', 0, 8, 8, 0, 0, 0, 19, 21, 93, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.504874');
INSERT INTO public.ubicazioni VALUES ('B    0 29  1', '9791280543080 ', 0, 6, 6, 0, 0, 0, 14, 11, 65, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.83811');
INSERT INTO public.ubicazioni VALUES ('B    0 11  1', '9788804758730 ', 0, 4, 4, 0, 0, 0, 12, 9, 47, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.238185');
INSERT INTO public.ubicazioni VALUES ('B    0 20  1', '9788869855320 ', 0, 5, 5, 0, 0, 0, 13, 12, 56, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0  7  1', '9788804777519 ', 0, 7, 7, 0, 0, 0, 17, 23, 79, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.246442');
INSERT INTO public.ubicazioni VALUES ('A    0 18  1', '9788817181556 ', 0, 2, 2, 0, 0, 0, 5, 2, 18, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.346577');
INSERT INTO public.ubicazioni VALUES ('B    0 35  1', '9791221205329 ', 0, 6, 6, 0, 0, 0, 15, 9, 71, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.05471');
INSERT INTO public.ubicazioni VALUES ('C    0 10  1', '9791254720738 ', 0, 7, 7, 0, 0, 0, 18, 20, 82, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.488278');
INSERT INTO public.ubicazioni VALUES ('A    0 31  1', '9788804755852 ', 0, 3, 3, 0, 0, 0, 6, 5, 31, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.879709');
INSERT INTO public.ubicazioni VALUES ('A    0 15  1', '9788804756644 ', 0, 2, 2, 0, 0, 0, 4, 5, 15, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.929663');
INSERT INTO public.ubicazioni VALUES ('B    0  9  1', '9788806257309 ', 0, 4, 4, 0, 0, 0, 11, 15, 45, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.43829');
INSERT INTO public.ubicazioni VALUES ('X    0 85  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1085, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  2  2', '', 0, 10, 10, 0, 0, 0, 19, 27, 115, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('B    0 10  1', '9788804770183 ', 0, 4, 4, 0, 0, 0, 12, 10, 46, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.15477');
INSERT INTO public.ubicazioni VALUES ('X    0 81  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1081, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 62  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1062, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 87  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1087, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 13  3', '', 0, 10, 10, 0, 0, 0, 29, 30, 158, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  7  2', '', 0, 10, 10, 0, 0, 0, 18, 27, 135, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 63  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1063, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 82  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1082, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0  5  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1005, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.979718');
INSERT INTO public.ubicazioni VALUES ('X    0 53  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1053, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 27  1', '', 0, 11, 11, 0, 0, 0, 27, 27, 183, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 60  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1060, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 93  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1093, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 70  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1070, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 12  3', '', 0, 10, 10, 0, 0, 0, 23, 26, 154, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 15  3', '', 0, 10, 10, 0, 0, 0, 30, 30, 166, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 34  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1034, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  9  3', '', 0, 10, 10, 0, 0, 0, 21, 30, 142, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 13  4', '', 0, 10, 10, 0, 0, 0, 29, 29, 157, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 72  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1072, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 47  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1047, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 61  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1061, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 33  1', '', 0, 12, 12, 0, 0, 0, 28, 25, 189, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 11  2', '', 0, 10, 10, 0, 0, 0, 22, 31, 151, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('A    0 12  1', '9791221206272 ', 0, 1, 1, 0, 0, 0, 2, 4, 12, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0 25  1', '9791254720882 ', 0, 9, 9, 0, 0, 0, 20, 17, 97, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('A    0 36  1', '9791221206593 ', 0, 3, 3, 0, 0, 0, 7, 4, 36, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 49  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1049, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('B    0 33  1', '9788804758242 ', 0, 6, 6, 0, 0, 0, 14, 15, 69, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.354827');
INSERT INTO public.ubicazioni VALUES ('X    0 99  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1099, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 69  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1069, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('B    0  5  1', '9791221206609 ', 0, 4, 4, 0, 0, 0, 11, 11, 41, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.138174');
INSERT INTO public.ubicazioni VALUES ('D    0  8  2', '', 0, 10, 10, 0, 0, 0, 23, 31, 139, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('A    0 10  1', '9788817178877 ', 0, 1, 1, 0, 0, 0, 2, 2, 10, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.971428');
INSERT INTO public.ubicazioni VALUES ('X    0 37  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1037, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0 18  1', '9788866407911 ', 0, 8, 8, 0, 0, 0, 19, 20, 90, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.329822');
INSERT INTO public.ubicazioni VALUES ('X    0  8  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1008, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 23  1', '', 0, 11, 11, 0, 0, 0, 26, 31, 179, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('A    0 34  1', '9791221206586 ', 0, 3, 3, 0, 0, 0, 7, 2, 34, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.763026');
INSERT INTO public.ubicazioni VALUES ('A    0  9  1', '9788820074562 ', 0, 1, 1, 0, 0, 0, 1, 7, 9, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.271444');
INSERT INTO public.ubicazioni VALUES ('D    0  7  3', '', 0, 10, 10, 0, 0, 0, 18, 26, 134, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('B    0  2  1', '9788820077785 ', 0, 4, 4, 0, 0, 0, 9, 9, 38, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.146447');
INSERT INTO public.ubicazioni VALUES ('C    0 32  1', '', 0, 9, 9, 0, 0, 0, 21, 18, 104, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0  1  1', '9788836161805 ', 0, 7, 7, 0, 0, 0, 17, 17, 73, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.871442');
INSERT INTO public.ubicazioni VALUES ('A    0 24  1', '9788891589750 ', 0, 2, 2, 0, 0, 0, 5, 8, 24, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 15  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1015, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('B    0 16  1', '9788855442114 ', 0, 5, 5, 0, 0, 0, 12, 16, 52, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.121428');
INSERT INTO public.ubicazioni VALUES ('D    0  4  4', '', 0, 10, 10, 0, 0, 0, 20, 25, 121, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('A    0 33  1', '9788856689877 ', 0, 3, 3, 0, 0, 0, 6, 7, 33, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.421526');
INSERT INTO public.ubicazioni VALUES ('X    0 16  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1016, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('B    0 12  1', '9788804775126 ', 0, 4, 4, 0, 0, 0, 12, 12, 48, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.113187');
INSERT INTO public.ubicazioni VALUES ('B    0 31  1', '9791221205336 ', 0, 6, 6, 0, 0, 0, 14, 13, 67, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.088119');
INSERT INTO public.ubicazioni VALUES ('A    0 17  1', '9781803709376 ', 0, 2, 2, 0, 0, 0, 4, 7, 17, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.104885');
INSERT INTO public.ubicazioni VALUES ('D    0  9  4', '', 0, 10, 10, 0, 0, 0, 21, 29, 141, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 78  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1078, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 10  2', '', 0, 10, 10, 0, 0, 0, 22, 27, 147, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0 23  1', '9791254720769 ', 0, 8, 8, 0, 0, 0, 19, 23, 95, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.988108');
INSERT INTO public.ubicazioni VALUES ('D    0 15  4', '', 0, 10, 10, 0, 0, 0, 30, 29, 165, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  7  1', '', 0, 10, 10, 0, 0, 0, 18, 28, 136, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 86  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1086, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 10  4', '', 0, 10, 10, 0, 0, 0, 22, 25, 145, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('A    0 20  1', '9788817181853 ', 0, 2, 2, 0, 0, 0, 5, 4, 20, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.396523');
INSERT INTO public.ubicazioni VALUES ('B    0 18  1', '9788820078270 ', 0, 5, 5, 0, 0, 0, 13, 10, 54, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.896403');
INSERT INTO public.ubicazioni VALUES ('C    0  5  1', '9788806259280 ', 0, 7, 7, 0, 0, 0, 17, 21, 77, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.796401');
INSERT INTO public.ubicazioni VALUES ('A    0  5  1', '9791221206265 ', 0, 1, 1, 0, 0, 0, 1, 3, 5, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.013127');
INSERT INTO public.ubicazioni VALUES ('D    0  5  4', '', 0, 10, 10, 0, 0, 0, 17, 25, 125, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0 26  1', '9788869855344 ', 0, 9, 9, 0, 0, 0, 20, 20, 98, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('A    0 27  1', '9791221206647 ', 0, 3, 3, 0, 0, 0, 6, 1, 27, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.413286');
INSERT INTO public.ubicazioni VALUES ('D    0 20  1', '', 0, 10, 10, 0, 0, 0, 26, 30, 176, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  1  2', '', 0, 10, 10, 0, 0, 0, 18, 31, 111, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0 15  1', '9788866407850 ', 0, 8, 8, 0, 0, 0, 18, 23, 87, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.496535');
INSERT INTO public.ubicazioni VALUES ('C    0 33  1', '', 0, 9, 9, 0, 0, 0, 21, 17, 105, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0  2  1', '9788891837974 ', 0, 7, 7, 0, 0, 0, 17, 20, 74, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.179756');
INSERT INTO public.ubicazioni VALUES ('C    0 29  1', '9791254720615 ', 0, 9, 9, 0, 0, 0, 20, 21, 101, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('B    0 25  1', '9788806254575 ', 0, 6, 6, 0, 0, 0, 13, 15, 61, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.363231');
INSERT INTO public.ubicazioni VALUES ('A    0 21  1', '9788891837615 ', 0, 2, 2, 0, 0, 0, 5, 3, 21, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.213166');
INSERT INTO public.ubicazioni VALUES ('X    0 29  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1029, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 64  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1064, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 51  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1051, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 39  1', '', 0, 12, 12, 0, 0, 0, 28, 31, 195, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 38  1', '', 0, 12, 12, 0, 0, 0, 28, 32, 194, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 18  1', '', 0, 10, 10, 0, 0, 0, 26, 28, 174, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  4  3', '', 0, 10, 10, 0, 0, 0, 20, 26, 122, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('A    0 16  1', '9788806258382 ', 0, 2, 2, 0, 0, 0, 4, 8, 16, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.829779');
INSERT INTO public.ubicazioni VALUES ('A    0  6  1', '9788806258399 ', 0, 1, 1, 0, 0, 0, 1, 6, 6, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.388283');
INSERT INTO public.ubicazioni VALUES ('A    0 11  1', '9791221206876 ', 0, 1, 1, 0, 0, 0, 2, 1, 11, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.071406');
INSERT INTO public.ubicazioni VALUES ('A    0 25  1', '9781803709383 ', 0, 3, 3, 0, 0, 0, 5, 7, 25, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.196436');
INSERT INTO public.ubicazioni VALUES ('A    0 13  1', '9788804774549 ', 0, 2, 2, 0, 0, 0, 2, 3, 13, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.371488');
INSERT INTO public.ubicazioni VALUES ('C    0 20  1', '9791254720486 ', 0, 8, 8, 0, 0, 0, 19, 22, 92, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.863086');
INSERT INTO public.ubicazioni VALUES ('B    0 34  1', '9788836162383 ', 0, 6, 6, 0, 0, 0, 15, 10, 70, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.471597');
INSERT INTO public.ubicazioni VALUES ('B    0 24  1', '9788806257699 ', 0, 5, 5, 0, 0, 0, 13, 16, 60, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 12  1', '', 0, 10, 10, 0, 0, 0, 23, 28, 156, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  3  4', '', 0, 10, 10, 0, 0, 0, 19, 29, 117, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0 14  1', '9788855441537 ', 0, 8, 8, 0, 0, 0, 18, 24, 86, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.93807');
INSERT INTO public.ubicazioni VALUES ('X    0  1  1', '9788804984412 ', 0, 0, 0, 0, 0, 0, 0, 0, 1001, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.096425');
INSERT INTO public.ubicazioni VALUES ('X    0 80  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1080, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0  8  1', '9791221206425 ', 0, 7, 7, 0, 0, 0, 18, 18, 80, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.30482');
INSERT INTO public.ubicazioni VALUES ('D    0 16  1', '', 0, 10, 10, 0, 0, 0, 31, 28, 172, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('B    0 28  1', '9788806258627 ', 0, 6, 6, 0, 0, 0, 14, 12, 64, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.463275');
INSERT INTO public.ubicazioni VALUES ('X    0 97  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1097, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0  4  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1004, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.921406');
INSERT INTO public.ubicazioni VALUES ('D    0 16  2', '', 0, 10, 10, 0, 0, 0, 31, 27, 171, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 14  4', '', 0, 10, 10, 0, 0, 0, 30, 25, 161, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 16  3', '', 0, 10, 10, 0, 0, 0, 31, 26, 170, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 12  2', '', 0, 10, 10, 0, 0, 0, 23, 27, 155, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 12  4', '', 0, 10, 10, 0, 0, 0, 23, 25, 153, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 14  3', '', 0, 10, 10, 0, 0, 0, 30, 26, 162, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  8  4', '', 0, 10, 10, 0, 0, 0, 23, 29, 137, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  5  3', '', 0, 10, 10, 0, 0, 0, 17, 26, 126, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('B    0 32  1', '9788804764847 ', 0, 6, 6, 0, 0, 0, 14, 16, 68, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.729747');
INSERT INTO public.ubicazioni VALUES ('A    0 14  1', '9791221205404 ', 0, 2, 2, 0, 0, 0, 2, 5, 14, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.779725');
INSERT INTO public.ubicazioni VALUES ('A    0  8  1', '9788856688269 ', 0, 1, 1, 0, 0, 0, 1, 8, 8, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.404863');
INSERT INTO public.ubicazioni VALUES ('B    0 15  1', '9788804776154 ', 0, 5, 5, 0, 0, 0, 12, 13, 51, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.446545');
INSERT INTO public.ubicazioni VALUES ('B    0 17  1', '9788806216566 ', 0, 5, 5, 0, 0, 0, 12, 15, 53, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.163176');
INSERT INTO public.ubicazioni VALUES ('X    0  7  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1007, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  9  2', '', 0, 10, 10, 0, 0, 0, 21, 31, 143, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0  4  1', '9788817181587 ', 0, 7, 7, 0, 0, 0, 17, 22, 76, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.479953');
INSERT INTO public.ubicazioni VALUES ('C    0  6  1', '9791221206432 ', 0, 7, 7, 0, 0, 0, 17, 24, 78, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.85475');
INSERT INTO public.ubicazioni VALUES ('D    0  5  2', '', 0, 10, 10, 0, 0, 0, 17, 27, 127, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 10  3', '', 0, 10, 10, 0, 0, 0, 22, 26, 146, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 13  1', '', 0, 10, 10, 0, 0, 0, 29, 32, 160, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 17  1', '', 0, 10, 10, 0, 0, 0, 26, 25, 173, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 84  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1084, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 92  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1092, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 30  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1030, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 25  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1025, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 27  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1027, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  1  1', '', 0, 10, 10, 0, 0, 0, 18, 32, 112, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  3  1', '', 0, 10, 10, 0, 0, 0, 19, 32, 120, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 59  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1059, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 58  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1058, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 88  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1088, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 14  1', '', 0, 10, 10, 0, 0, 0, 30, 28, 164, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 75  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1075, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 54  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1054, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 96  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1096, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 68  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1068, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 32  1', '', 0, 11, 11, 0, 0, 0, 28, 26, 188, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  6  4', '', 0, 10, 10, 0, 0, 0, 17, 29, 129, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  6  3', '', 0, 10, 10, 0, 0, 0, 17, 30, 130, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  6  2', '', 0, 10, 10, 0, 0, 0, 17, 31, 131, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0  9  1', '', 0, 10, 10, 0, 0, 0, 21, 32, 144, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 11  3', '', 0, 10, 10, 0, 0, 0, 22, 30, 150, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 35  1', '', 0, 12, 12, 0, 0, 0, 28, 27, 191, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 28  1', '', 0, 11, 11, 0, 0, 0, 27, 30, 184, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 25  1', '', 0, 11, 11, 0, 0, 0, 27, 25, 181, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 24  1', '', 0, 11, 11, 0, 0, 0, 27, 26, 180, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 31  1', '', 0, 11, 11, 0, 0, 0, 27, 31, 187, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0 31  1', '', 0, 9, 9, 0, 0, 0, 20, 23, 103, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0 24  1', '9788804774532 ', 0, 8, 8, 0, 0, 0, 20, 18, 96, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.913067');
INSERT INTO public.ubicazioni VALUES ('C    0 13  1', '9788866408208 ', 0, 8, 8, 0, 0, 0, 18, 21, 85, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.204759');
INSERT INTO public.ubicazioni VALUES ('C    0 17  1', '9788866407928 ', 0, 8, 8, 0, 0, 0, 19, 17, 89, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.263178');
INSERT INTO public.ubicazioni VALUES ('C    0 16  1', '9788817182225 ', 0, 8, 8, 0, 0, 0, 19, 18, 88, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.221439');
INSERT INTO public.ubicazioni VALUES ('D    0 26  1', '', 0, 11, 11, 0, 0, 0, 27, 28, 182, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 21  1', '', 0, 11, 11, 0, 0, 0, 26, 29, 177, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 29  1', '', 0, 11, 11, 0, 0, 0, 27, 29, 185, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 22  1', '', 0, 11, 11, 0, 0, 0, 26, 32, 178, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('D    0 30  1', '', 0, 11, 11, 0, 0, 0, 27, 32, 186, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0 35  1', '', 0, 9, 9, 0, 0, 0, 21, 19, 107, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0 34  1', '', 0, 9, 9, 0, 0, 0, 21, 20, 106, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0 28  1', '9791254720820 ', 0, 9, 9, 0, 0, 0, 20, 22, 100, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('C    0  9  1', '9788804772415 ', 0, 7, 7, 0, 0, 0, 18, 17, 81, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.754759');
INSERT INTO public.ubicazioni VALUES ('D    0  1  3', '', 0, 10, 10, 0, 0, 0, 18, 30, 110, 0, 0, 0, 0, 'A', 'N', 'S', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('X    0 35  1', '', 0, 0, 0, 0, 0, 0, 0, 0, 1035, 0, 0, 0, 0, 'M', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('B    0  1  1', '9791221206555 ', 0, 4, 4, 0, 0, 0, 11, 10, 37, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.29648');
INSERT INTO public.ubicazioni VALUES ('B    0 21  1', '9791221204889 ', 0, 5, 5, 0, 0, 0, 13, 11, 57, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.454869');
INSERT INTO public.ubicazioni VALUES ('A    0 22  1', '9791221206326 ', 0, 2, 2, 0, 0, 0, 5, 6, 22, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.029707');
INSERT INTO public.ubicazioni VALUES ('B    0  8  1', '9788854051980 ', 0, 4, 4, 0, 0, 0, 11, 16, 44, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.672169');
INSERT INTO public.ubicazioni VALUES ('A    0 28  1', '9791221204698 ', 0, 3, 3, 0, 0, 0, 6, 4, 28, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.996414');
INSERT INTO public.ubicazioni VALUES ('B    0 23  1', '9788804764601 ', 0, 5, 5, 0, 0, 0, 13, 13, 59, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.84644');
INSERT INTO public.ubicazioni VALUES ('A    0  7  1', '9788804751939 ', 0, 1, 1, 0, 0, 0, 1, 5, 7, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.746435');
INSERT INTO public.ubicazioni VALUES ('A    0  3  1', '9788817182126 ', 0, 1, 1, 0, 0, 0, 1, 1, 3, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.279718');
INSERT INTO public.ubicazioni VALUES ('B    0 30  1', '9788817175425 ', 0, 6, 6, 0, 0, 0, 14, 14, 66, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.021418');
INSERT INTO public.ubicazioni VALUES ('A    0 26  1', '9788891838360 ', 0, 3, 3, 0, 0, 0, 6, 2, 26, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.063133');
INSERT INTO public.ubicazioni VALUES ('B    0 27  1', '9791221204872 ', 0, 6, 6, 0, 0, 0, 14, 9, 63, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.171434');
INSERT INTO public.ubicazioni VALUES ('A    0 30  1', '9791221204704 ', 0, 3, 3, 0, 0, 0, 6, 6, 30, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.379943');
INSERT INTO public.ubicazioni VALUES ('B    0 22  1', '9788856689853 ', 0, 5, 5, 0, 0, 0, 13, 14, 58, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.129768');
INSERT INTO public.ubicazioni VALUES ('A    0 23  1', '9791221206630 ', 0, 2, 2, 0, 0, 0, 5, 5, 23, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.046403');
INSERT INTO public.ubicazioni VALUES ('B    0  4  1', '9788806257446 ', 0, 4, 4, 0, 0, 0, 11, 12, 40, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:44.887965');
INSERT INTO public.ubicazioni VALUES ('A    0 35  1', '9788891839152 ', 0, 3, 3, 0, 0, 0, 7, 1, 35, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.254814');
INSERT INTO public.ubicazioni VALUES ('B    0  3  1', '9791221206579 ', 0, 4, 4, 0, 0, 0, 11, 9, 39, 0, 0, 0, 0, 'A', 'N', 'P', 'alberto', '2017-12-12 11:55:54.144702', 'alberto', '2018-01-28 14:29:45.430015');


--
-- Name: ubicazioni_prodotto; Type: INDEX; Schema: public; Owner: roberto
--

CREATE INDEX ubicazioni_prodotto ON public.ubicazioni USING btree (codprod);


--
-- Name: ubicazioni_ubicazione; Type: INDEX; Schema: public; Owner: roberto
--

CREATE UNIQUE INDEX ubicazioni_ubicazione ON public.ubicazioni USING btree (ubicazione);


--
-- PostgreSQL database dump complete
--

