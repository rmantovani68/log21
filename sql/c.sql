--
-- Selected TOC Entries:
--
\connect - roberto
--
-- TOC Entry ID 2 (OID 20009)
--
-- Name: ric_ord Type: TABLE Owner: roberto
--

CREATE TABLE "ric_ord" (
	"ordprog" character(17),
	"rocdrid" character(7),
	"rocdspc" character(2),
	"rocdare" character(3),
	"rocdven" character(2),
	"rodscli" character(31),
	"rococli" character(31),
	"roincli" character(31),
	"rolocli" character(22),
	"roprcli" character(2),
	"rocpcli" character(5),
	"ronzcli" character(3),
	"rocdcla" character(7),
	"rocdspa" character(2),
	"rodscla" character(31),
	"roincla" character(31),
	"rococla" character(31),
	"rolocla" character(22),
	"roprcla" character(2),
	"rocpcla" character(5),
	"ronzcla" character(3),
	"roswspf" character(1),
	"ronmbam" character(6),
	"rodtbam" character(8),
	"rotptar" character(1),
	"roswcol" character(1),
	"rovlcon" bigint DEFAULT 0,
	"rotpspe" character(2),
	"rocdlin" character(2),
	"rocdsca" character(2),
	"rocdssc" character(2),
	"rocdsot" character(3),
	"rocdmov" character(4),
	"rotpdoc" character(2),
	"rodteva" character(8),
	"roswfin" character(1),
	"rocdfpg" character(3),
	"rotpbut" character(1),
	"roorins" character(8),
	"rocdval" character(4),
	"rovlcoe" bigint DEFAULT 0,
	"ronmced" character(6),
	"rocdve1" character(6),
	"rocdve2" character(6),
	"rocdve3" character(6),
	"roidvet" character(2),
	"ronmrgh" integer DEFAULT 0,
	"ronmrgp" integer DEFAULT 0,
	"ronmcpe" integer DEFAULT 0,
	"ronmcpp" integer DEFAULT 0,
	"ronmcll" integer DEFAULT 0,
	"ronmcla" integer DEFAULT 0,
	"rocdbtc" character(8),
	"ropspre" integer DEFAULT 0,
	"ropsrea" integer DEFAULT 0,
	"rotmrcz" timestamp with time zone,
	"rotmini" timestamp with time zone,
	"rotmeva" timestamp with time zone,
	"roaaxab" smallint DEFAULT 0,
	"roprxab" integer DEFAULT 0,
	"rodtxab" character(8),
	"ronmdis" integer DEFAULT 0,
	"rodtdis" character(8),
	"roprgln" integer DEFAULT 0,
	"rocdflg" character(1),
	"rostato" character(1),
	"ordtipo" character(1)
);

--
-- TOC Entry ID 3 (OID 20009)
--
-- Name: "ric_ord_rocdrid" Type: INDEX Owner: roberto
--

CREATE  INDEX "ric_ord_rocdrid" on "ric_ord" using btree ( "rocdrid" "bpchar_ops" );

--
-- TOC Entry ID 4 (OID 20009)
--
-- Name: "ric_ord_ordtipo" Type: INDEX Owner: roberto
--

CREATE  INDEX "ric_ord_ordtipo" on "ric_ord" using btree ( "ordtipo" "bpchar_ops" );

--
-- TOC Entry ID 5 (OID 20009)
--
-- Name: "ric_ord_rotpspe" Type: INDEX Owner: roberto
--

CREATE  INDEX "ric_ord_rotpspe" on "ric_ord" using btree ( "rotpspe" "bpchar_ops" );

--
-- TOC Entry ID 6 (OID 20009)
--
-- Name: "ric_ord_rocdlin" Type: INDEX Owner: roberto
--

CREATE  INDEX "ric_ord_rocdlin" on "ric_ord" using btree ( "rocdlin" "bpchar_ops" );

--
-- TOC Entry ID 7 (OID 20009)
--
-- Name: "ric_ord_rostato" Type: INDEX Owner: roberto
--

CREATE  INDEX "ric_ord_rostato" on "ric_ord" using btree ( "rostato" "bpchar_ops" );

--
-- TOC Entry ID 8 (OID 20009)
--
-- Name: "ric_ord_ordprog" Type: INDEX Owner: roberto
--

CREATE UNIQUE INDEX "ric_ord_ordprog" on "ric_ord" using btree ( "ordprog" "bpchar_ops" );

--
-- Selected TOC Entries:
--
\connect - roberto
--
-- TOC Entry ID 2 (OID 20210)
--
-- Name: col_prod Type: TABLE Owner: roberto
--

CREATE TABLE "col_prod" (
	"ordprog" character(17),
	"cpnmcol" integer DEFAULT 0,
	"cpnmppt" character(6),
	"cppspre" integer DEFAULT 0,
	"cppsrea" integer DEFAULT 0,
	"cpvlaff" integer DEFAULT 0,
	"cptpfor" character(1),
	"cpswfps" character(1),
	"cpswchm" character(1),
	"cpvlafe" bigint DEFAULT 0,
	"cpswlin" character(1),
	"cpnmrgh" smallint DEFAULT 0,
	"cpnmcpe" smallint DEFAULT 0,
	"cpbrcde" character(14),
	"cptmeva" timestamp with time zone,
	"cpcdflg" character(1),
	"cpstato" character(1),
	"cpcntsc" integer DEFAULT 0
);

--
-- TOC Entry ID 3 (OID 20210)
--
-- Name: "col_prod_ordprog" Type: INDEX Owner: roberto
--

CREATE  INDEX "col_prod_ordprog" on "col_prod" using btree ( "ordprog" "bpchar_ops" );

--
-- TOC Entry ID 4 (OID 20210)
--
-- Name: "col_prod_cpnmcol" Type: INDEX Owner: roberto
--

CREATE  INDEX "col_prod_cpnmcol" on "col_prod" using btree ( "cpnmcol" "int4_ops" );

--
-- TOC Entry ID 5 (OID 20210)
--
-- Name: "col_prod_cpbrcde" Type: INDEX Owner: roberto
--

CREATE  INDEX "col_prod_cpbrcde" on "col_prod" using btree ( "cpbrcde" "bpchar_ops" );

--
-- TOC Entry ID 6 (OID 20210)
--
-- Name: "col_prod_cpstato" Type: INDEX Owner: roberto
--

CREATE  INDEX "col_prod_cpstato" on "col_prod" using btree ( "cpstato" "bpchar_ops" );

--
-- TOC Entry ID 7 (OID 20210)
--
-- Name: "col_prod_cpswlin" Type: INDEX Owner: roberto
--

CREATE  INDEX "col_prod_cpswlin" on "col_prod" using btree ( "cpswlin" "bpchar_ops" );

--
-- Selected TOC Entries:
--
\connect - roberto
--
-- TOC Entry ID 2 (OID 20245)
--
-- Name: rig_prod Type: TABLE Owner: roberto
--

CREATE TABLE "rig_prod" (
	"ordprog" character(17),
	"rpnmcol" integer DEFAULT 0,
	"rpcdpro" character(12),
	"oldrow" character(3),
	"rpqtspe" integer DEFAULT 0,
	"rpqtgsp" integer DEFAULT 0,
	"rpcdflg" character(1),
	"rpqtord" integer DEFAULT 0,
	"rpqtdsp" integer DEFAULT 0,
	"rpcdubi" character(12),
	"rpprior" smallint DEFAULT 0,
	"rpprrig" integer DEFAULT 0,
	"rpswffo" smallint DEFAULT 0,
	"rpswpcf" smallint DEFAULT 0,
	"rpcdopr" character(2),
	"rptmpre" timestamp with time zone,
	"rpstato" character(1),
	"rpprrow" smallint,
	"dummy" text,
	"rppzpre" bigint
);

--
-- TOC Entry ID 3 (OID 20245)
--
-- Name: "rig_prod_rpcdflg" Type: INDEX Owner: roberto
--

CREATE  INDEX "rig_prod_rpcdflg" on "rig_prod" using btree ( "rpcdflg" "bpchar_ops" );

--
-- TOC Entry ID 4 (OID 20245)
--
-- Name: "rig_prod_rpcdpro" Type: INDEX Owner: roberto
--

CREATE  INDEX "rig_prod_rpcdpro" on "rig_prod" using btree ( "rpcdpro" "bpchar_ops" );

--
-- TOC Entry ID 5 (OID 20245)
--
-- Name: "rig_prod_rpprrig" Type: INDEX Owner: roberto
--

CREATE  INDEX "rig_prod_rpprrig" on "rig_prod" using btree ( "rpprrig" "int4_ops" );

--
-- TOC Entry ID 6 (OID 20245)
--
-- Name: "rig_prod_rpstato" Type: INDEX Owner: roberto
--

CREATE  INDEX "rig_prod_rpstato" on "rig_prod" using btree ( "rpstato" "bpchar_ops" );

--
-- TOC Entry ID 7 (OID 20245)
--
-- Name: "rig_prod_rpcdubi" Type: INDEX Owner: roberto
--

CREATE  INDEX "rig_prod_rpcdubi" on "rig_prod" using btree ( "rpcdubi" "bpchar_ops" );

--
-- TOC Entry ID 8 (OID 20245)
--
-- Name: "rig_prod_ordprog_rpnmcol_rpcdub" Type: INDEX Owner: roberto
--

CREATE  INDEX "rig_prod_ordprog_rpnmcol_rpcdub" on "rig_prod" using btree ( "ordprog" "bpchar_ops", "rpnmcol" "int4_ops", "rpcdubi" "bpchar_ops" );

--
-- TOC Entry ID 9 (OID 20245)
--
-- Name: "rig_prod_ordprog" Type: INDEX Owner: roberto
--

CREATE  INDEX "rig_prod_ordprog" on "rig_prod" using btree ( "ordprog" "bpchar_ops" );

--
-- TOC Entry ID 10 (OID 20245)
--
-- Name: "rig_prod_rpnmcol" Type: INDEX Owner: roberto
--

CREATE  INDEX "rig_prod_rpnmcol" on "rig_prod" using btree ( "rpnmcol" "int4_ops" );

