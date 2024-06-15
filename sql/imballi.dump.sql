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
-- Name: imballi; Type: TABLE; Schema: public; Owner: luca; Tablespace: 
--

CREATE TABLE imballi (
    codice character(1),
    descriz character(30),
    lunghezza integer DEFAULT 0,
    altezza integer DEFAULT 0,
    larghezza integer DEFAULT 0,
    tara integer DEFAULT 0,
    pesomax integer DEFAULT 0,
    volperc smallint DEFAULT 0,
    tplavor smallint DEFAULT 0,
    ordtipo character(1),
    categoria smallint DEFAULT 0
);



--
-- Data for Name: imballi; Type: TABLE DATA; Schema: public; Owner: luca
--

COPY imballi (codice, descriz, lunghezza, altezza, larghezza, tara, pesomax, volperc, tplavor, ordtipo, categoria) FROM stdin;
8	grande                        	590	240	240	510	20000	80	1	N	0
7	piccolo                       	295	240	240	260	17000	80	1	N	0
F	TIPO IMBALLO FUORI FORMATO    	0	0	0	0	0	0	1	N	\N
A	Fuori Formato A categoria 1   	1200	1400	800	12000	500000	100	1	N	1
C	TIPO IMBALLO PRECONFEZIONATO  	0	0	0	0	0	100	1	N	0
P	TIPO IMBALLO PALLET           	1200	1400	800	10000	450000	100	1	N	0
1	TIPO IMBALLO 1                	395	340	295	635	21000	80	0	N	0
2	TIPO IMBALLO 2                	395	228	295	460	20000	75	0	N	0
3	TIPO IMBALLO 3                	395	100	288	333	17000	65	0	N	0
\.


--
-- Name: imballi_codimb; Type: INDEX; Schema: public; Owner: luca; Tablespace: 
--

CREATE INDEX imballi_codimb ON imballi USING btree (codice);


--
-- Name: imballi_univoco; Type: INDEX; Schema: public; Owner: luca; Tablespace: 
--

CREATE UNIQUE INDEX imballi_univoco ON imballi USING btree (codice, ordtipo);


--
-- PostgreSQL database dump complete
--

