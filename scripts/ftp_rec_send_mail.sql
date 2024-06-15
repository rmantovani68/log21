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

--
-- Name: ftp_send_log_sequence; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE ftp_send_log_sequence
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


--
-- PostgreSQL database dump complete
--

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
-- Name: ftp_send_log; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE ftp_send_log (
    id integer DEFAULT nextval('ftp_send_log_sequence'::regclass),
    code integer,
    message text,
    "time" timestamp without time zone DEFAULT now()
);


--
-- PostgreSQL database dump complete
--

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

--
-- Name: ftp_receive_log_sequence; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE ftp_receive_log_sequence
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


--
-- PostgreSQL database dump complete
--

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

--
-- Name: mail_sender_log_sequence; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE mail_sender_log_sequence
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


--
-- PostgreSQL database dump complete
--

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
-- Name: mail_sender_log; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE mail_sender_log (
    id integer DEFAULT nextval('mail_sender_log_sequence'::regclass),
    code integer,
    message text,
    "time" timestamp without time zone DEFAULT now()
);


--
-- PostgreSQL database dump complete
--

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
-- Name: tasks; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE tasks (
    id_task integer NOT NULL,
    nome_servente text,
    pid_servente text,
    nome_cliente text,
    pid_cliente text,
    codice_tipo_task integer DEFAULT 0,
    stringa_parametri text,
    stato text
);


--
-- Name: tasks_id_task_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE tasks_id_task_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


--
-- Name: tasks_id_task_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE tasks_id_task_seq OWNED BY tasks.id_task;


--
-- Name: id_task; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY tasks ALTER COLUMN id_task SET DEFAULT nextval('tasks_id_task_seq'::regclass);


--
-- PostgreSQL database dump complete
--

