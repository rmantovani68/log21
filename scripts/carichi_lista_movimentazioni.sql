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
-- Name: carichi_lista_movimentazioni; Type: TABLE; Schema: public; Owner: alberto; Tablespace: 
--

CREATE TABLE carichi_lista_movimentazioni (
    mocdpro text,
    ctdstit text,
    monmcpe integer,
    modscau text,
    monmdis integer,
    mocdudc text,
    motmmov timestamp without time zone,
    moprmov integer
);


ALTER TABLE public.carichi_lista_movimentazioni OWNER TO alberto;

--
-- Data for Name: carichi_lista_movimentazioni; Type: TABLE DATA; Schema: public; Owner: alberto
--

COPY carichi_lista_movimentazioni (mocdpro, ctdstit, monmcpe, modscau, monmdis, mocdudc, motmmov, moprmov) FROM stdin;
000005402692	Tea sommelier	421	SPEDIZIONE	0		2015-01-09 08:23:42.661835	208734
000005402692	Tea sommelier	58	SPEDIZIONE	0		2015-01-09 11:03:42.944879	208992
000005402692	Tea sommelier	19	SPEDIZIONE	0		2015-01-09 11:23:40.715686	209053
000005402692	Tea sommelier	210	CONTEGGIO	0	0436137	2015-01-09 14:31:50.398661	209183
000005402692	Tea sommelier	210	RESTITUZIONE	2376	0436137	2015-01-10 10:31:57.86956	209336
\.


--
-- PostgreSQL database dump complete
--

