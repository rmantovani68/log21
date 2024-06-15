--
-- PostgreSQL database dump
--

SET statement_timeout = 0;
SET lock_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;

SET search_path = public, pg_catalog;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: settori; Type: TABLE; Schema: public; Owner: luca; Tablespace: 
--

CREATE TABLE settori (
    settore smallint DEFAULT 0,
    isola smallint DEFAULT 0,
    flag smallint DEFAULT 0,
    tipo smallint DEFAULT 0,
    operatore character(2),
    cedola integer DEFAULT 0
);



--
-- Data for Name: settori; Type: TABLE DATA; Schema: public; Owner: luca
--

COPY settori (settore, isola, flag, tipo, operatore, cedola) FROM stdin;
12	0	1	1	  	39
11	0	1	2	  	2
10	0	1	2	  	19
9	0	1	2	  	24
8	0	1	1	  	24
7	0	1	1	  	24
6	0	1	1	10	24
5	0	1	1	04	24
4	0	1	1	17	24
3	0	1	1	06	24
2	0	1	1	12	24
1	0	1	0	02	24
\.


--
-- PostgreSQL database dump complete
--

