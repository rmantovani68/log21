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
-- Name: operatori; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE operatori (
    opcdope character(2),
    opnmope character(20),
    opcgope character(20),
    opinope character(30),
    oploope character(30),
    opprope character(2),
    opcpope character(5),
    opnmtel character(20),
    oplogin character(10),
    oppassw character(10)
);


--
-- Data for Name: operatori; Type: TABLE DATA; Schema: public; Owner: -
--

COPY operatori (opcdope, opnmope, opcgope, opinope, oploope, opprope, opcpope, opnmtel, oplogin, oppassw) FROM stdin;
02	ROBERTA             	PARMA               	                              	                              	  	     	3400689643          	\N	\N
01	SARA                	LUI                 	                              	                              	  	     	3420308046          	\N	\N
03	ISKRA               	ALEXOVA             	                              	                              	  	     	3491368415          	\N	\N
04	SLAVINKA            	RUZIC               	                              	                              	  	     	3921576751          	\N	\N
06	LEONELA             	BUONINCONTRI        	                              	                              	  	     	3209360583          	\N	\N
07	DANIELA             	SAVA                	                              	                              	  	     	3895117877          	\N	\N
13	GIANFRANCO          	MATTUZZI            	                              	                              	  	     	3469537221          	\N	\N
05	JONELA              	FILIP STEFAN        	                              	                              	  	     	3474903530          	\N	\N
08	CATALINA            	COJOCARIU           	                              	                              	  	     	3272067508          	\N	\N
09	CRISTINA            	PARVULESCU          	                              	                              	  	     	3281310436          	\N	\N
10	IRINA               	MOTOLEA             	                              	                              	  	     	3407130698          	\N	\N
12	JULIAN              	FILIP STEFAN        	                              	                              	  	     	3208978173          	\N	\N
14	VALENTINA           	NIKOLIC             	                              	                              	  	     	3205659430          	\N	\N
11	MARIA               	FILIP               	                              	                              	  	     	3936782582          	\N	\N
15	VALERIA             	BONVICINI           	                              	                              	  	     	3463657742          	\N	\N
16	RALUCA              	MANEA               	                              	                              	  	     	3280889457          	\N	\N
\.


--
-- Name: operatori_key; Type: INDEX; Schema: public; Owner: -; Tablespace: 
--

CREATE UNIQUE INDEX operatori_key ON operatori USING btree (opcdope);


--
-- PostgreSQL database dump complete
--

