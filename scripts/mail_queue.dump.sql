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
-- Name: mail_queue; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE mail_queue (
    mqprogr integer NOT NULL,
    mqstato text,
    mqtoadd text,
    mqccadd text,
    mqbcadd text,
    mqrtadd text,
    mqsubjt text,
    mqtbody text,
    mqattac text,
    mqtmins timestamp without time zone,
    mqtminv timestamp without time zone,
    mqcderr text,
    mqcmout text,
    mqdestn text,
    mqprior integer DEFAULT 100
);


--
-- Name: mail_queue_mqprogr_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE mail_queue_mqprogr_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


--
-- Name: mail_queue_mqprogr_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE mail_queue_mqprogr_seq OWNED BY mail_queue.mqprogr;


--
-- Name: mqprogr; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY mail_queue ALTER COLUMN mqprogr SET DEFAULT nextval('mail_queue_mqprogr_seq'::regclass);


--
-- Name: mail_queue_key; Type: INDEX; Schema: public; Owner: -; Tablespace: 
--

CREATE UNIQUE INDEX mail_queue_key ON mail_queue USING btree (mqprogr);


--
-- PostgreSQL database dump complete
--

