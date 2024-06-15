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
-- Name: displays; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE displays (
    display smallint DEFAULT 0,
    isola smallint DEFAULT 0,
    settore character(2),
    ios smallint DEFAULT 0,
    cpu smallint DEFAULT 0,
    modulo smallint DEFAULT 0
);


--
-- Data for Name: displays; Type: TABLE DATA; Schema: public; Owner: -
--

COPY displays (display, isola, settore, ios, cpu, modulo) FROM stdin;
2	0	2 	0	0	2
3	0	3 	0	0	3
4	0	4 	0	0	4
5	0	5 	0	0	5
6	0	6 	0	0	6
7	0	7 	0	0	7
8	0	8 	0	0	8
9	0	9 	0	0	9
10	0	10	0	0	10
11	0	11	0	0	11
12	0	12	0	0	12
1	0	1 	0	0	14
\.


--
-- Name: displays_key; Type: INDEX; Schema: public; Owner: -; Tablespace: 
--

CREATE UNIQUE INDEX displays_key ON displays USING btree (isola, display);


--
-- PostgreSQL database dump complete
--

