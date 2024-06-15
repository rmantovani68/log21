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
-- Name: ftp_sender_log; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE ftp_sender_log (
    id integer NOT NULL,
    code integer DEFAULT 0,
    message text,
    "time" timestamp without time zone DEFAULT now()
);


--
-- Name: ftp_sender_log_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE ftp_sender_log_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


--
-- Name: ftp_sender_log_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE ftp_sender_log_id_seq OWNED BY ftp_sender_log.id;


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY ftp_sender_log ALTER COLUMN id SET DEFAULT nextval('ftp_sender_log_id_seq'::regclass);


--
-- Name: ftp_sender_log_key; Type: INDEX; Schema: public; Owner: -; Tablespace: 
--

CREATE UNIQUE INDEX ftp_sender_log_key ON ftp_sender_log USING btree (id);


--
-- PostgreSQL database dump complete
--

