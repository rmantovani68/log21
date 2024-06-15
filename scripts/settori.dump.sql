--
-- PostgreSQL database dump
--

SET statement_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = off;
SET check_function_bodies = false;
SET client_min_messages = warning;
SET escape_string_warning = off;

SET search_path = public, pg_catalog;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: settori; Type: TABLE; Schema: public; Owner: -; Tablespace: 
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
-- Data for Name: settori; Type: TABLE DATA; Schema: public; Owner: -
--

COPY settori (settore, isola, flag, tipo, operatore, cedola) FROM stdin;
7	0	1	2	  	41
6	0	1	1	10	41
11	0	1	2	  	30
5	0	1	2	04	42
4	0	1	1	17	42
10	0	1	1	  	30
3	0	1	2	06	43
2	0	1	1	12	43
1	0	1	0	02	43
12	0	1	0	  	4303
9	0	1	2	  	39
8	0	1	1	  	39
\.


--
-- PostgreSQL database dump complete
--

