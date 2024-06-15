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

