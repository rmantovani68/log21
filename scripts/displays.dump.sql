--
-- Name: displays; Type: TABLE; Schema: public; Owner: roberto
--

-- CREATE TABLE public.displays (
--     dsnmdsp integer DEFAULT 0,
--     dsnmisl integer DEFAULT 0,
--     dsnmset text,
-- );


ALTER TABLE public.displays OWNER TO roberto;

--
-- Data for Name: displays; Type: TABLE DATA; Schema: public; Owner: roberto
--

INSERT INTO displays VALUES ( 1, 0, '1 ', 0, 0, 14, 'alberto', '2017-12-12 11:55:54.043281', NULL, NULL);
INSERT INTO displays VALUES ( 2, 0, '2 ', 0, 0, 2, 'alberto', '2017-12-12 11:55:54.043281', NULL, NULL);
INSERT INTO displays VALUES ( 3, 0, '3 ', 0, 0, 3, 'alberto', '2017-12-12 11:55:54.043281', NULL, NULL);
INSERT INTO displays VALUES ( 4, 0, '4 ', 0, 0, 4, 'alberto', '2017-12-12 11:55:54.043281', NULL, NULL);
INSERT INTO displays VALUES ( 5, 0, '5 ', 0, 0, 5, 'alberto', '2017-12-12 11:55:54.043281', NULL, NULL);
INSERT INTO displays VALUES ( 6, 0, '6 ', 0, 0, 6, 'alberto', '2017-12-12 11:55:54.043281', NULL, NULL);
INSERT INTO displays VALUES ( 7, 0, '7 ', 0, 0, 7, 'alberto', '2017-12-12 11:55:54.043281', NULL, NULL);
INSERT INTO displays VALUES ( 8, 0, '8 ', 0, 0, 8, 'alberto', '2017-12-12 11:55:54.043281', NULL, NULL);
INSERT INTO displays VALUES ( 9, 0, '9 ', 0, 0, 9, 'alberto', '2017-12-12 11:55:54.043281', NULL, NULL);
INSERT INTO displays VALUES (10, 0, '10', 0, 0, 10, 'alberto', '2017-12-12 11:55:54.043281', NULL, NULL);
INSERT INTO displays VALUES (11, 0, '11', 0, 0, 11, 'alberto', '2017-12-12 11:55:54.043281', NULL, NULL);
INSERT INTO displays VALUES (12, 0, '12', 0, 0, 12, 'alberto', '2017-12-12 11:55:54.043281', NULL, NULL);


--
-- Name: displays_key; Type: INDEX; Schema: public; Owner: roberto
--

CREATE UNIQUE INDEX displays_key ON public.displays USING btree (isola, display);


--
-- PostgreSQL database dump complete
--

