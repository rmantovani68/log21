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
-- Name: ftp_receive_log; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE ftp_receive_log (
    id integer DEFAULT nextval('ftp_receive_log_sequence'::regclass),
    code integer,
    message text,
    "time" timestamp without time zone DEFAULT now()
);


--
-- PostgreSQL database dump complete
--

