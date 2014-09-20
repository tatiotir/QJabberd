SET statement_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;

CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;

COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';

SET search_path = public, pg_catalog;

CREATE SEQUENCE contact_id_seq START WITH 1 INCREMENT BY 1 NO MINVALUE NO MAXVALUE CACHE 1;

ALTER TABLE public.contact_id_seq OWNER TO :table_user;

CREATE SEQUENCE metacontact_id_seq START WITH 1 INCREMENT BY 1 NO MINVALUE NO MAXVALUE CACHE 1;

ALTER TABLE public.metacontact_id_seq OWNER TO :table_user;

CREATE SEQUENCE offlinemessage_id_seq START WITH 1 INCREMENT BY 1 NO MINVALUE NO MAXVALUE CACHE 1;

ALTER TABLE public.offlinemessage_id_seq OWNER TO :table_user;

CREATE SEQUENCE offlinepresencesubscription_id_seq START WITH 1 INCREMENT BY 1 NO MINVALUE NO MAXVALUE CACHE 1;

ALTER TABLE public.offlinepresencesubscription_id_seq OWNER TO :table_user;

CREATE SEQUENCE privacylist_id_seq START WITH 1 INCREMENT BY 1 NO MINVALUE NO MAXVALUE CACHE 1;

ALTER TABLE public.privacylist_id_seq OWNER TO :table_user;

CREATE SEQUENCE privatestorage_id_seq START WITH 1 INCREMENT BY 1 NO MINVALUE NO MAXVALUE CACHE 1;

ALTER TABLE public.privatestorage_id_seq OWNER TO :table_user;

SET default_tablespace = '';

SET default_with_oids = false;

CREATE TABLE qjabberd_blocklist(user_id integer NOT NULL, jid character varying(510) NOT NULL);

ALTER TABLE public.qjabberd_blocklist OWNER TO :table_user;

CREATE TABLE qjabberd_contact(id integer DEFAULT nextval('contact_id_seq'::regclass) NOT NULL, user_id integer NOT NULL, approved integer, ask character varying(18) DEFAULT NULL::character varying, groups text, jid character varying(510) NOT NULL, name character varying(510) DEFAULT NULL::character varying, subscription character varying(8) DEFAULT NULL::character varying, version character varying(510) DEFAULT NULL::character varying);

ALTER TABLE public.qjabberd_contact OWNER TO :table_user;

CREATE TABLE qjabberd_metacontact(id integer DEFAULT nextval('metacontact_id_seq'::regclass) NOT NULL, user_id integer NOT NULL, jid character varying(510) NOT NULL, tag character varying(510) DEFAULT NULL::character varying, morder integer);

ALTER TABLE public.qjabberd_metacontact OWNER TO :table_user;

CREATE TABLE qjabberd_offlinemessage(id integer DEFAULT nextval('offlinemessage_id_seq'::regclass) NOT NULL, user_id integer NOT NULL, ufrom character varying(510) NOT NULL, stamp character varying(510) NOT NULL, otype character varying(510) NOT NULL, obody text NOT NULL);

ALTER TABLE public.qjabberd_offlinemessage OWNER TO :table_user;

CREATE TABLE qjabberd_offlinepresencesubscription(id integer DEFAULT nextval('offlinepresencesubscription_id_seq'::regclass) NOT NULL, user_id integer NOT NULL, stype character varying(24) NOT NULL, ufrom character varying(510) NOT NULL, uto character varying(510) NOT NULL, "presenceStanza" text NOT NULL);

ALTER TABLE public.qjabberd_offlinepresencesubscription OWNER TO :table_user;

CREATE TABLE qjabberd_privacylist(id integer DEFAULT nextval('privacylist_id_seq'::regclass) NOT NULL, user_id integer NOT NULL, ptype character varying(510) DEFAULT NULL::character varying, pvalue character varying(510) DEFAULT NULL::character varying, action character varying(510) DEFAULT NULL::character varying, iorder integer, child text, privacyListName character varying(510) DEFAULT NULL::character varying);

ALTER TABLE public.qjabberd_privacylist OWNER TO :table_user;

CREATE TABLE qjabberd_privatestorage(id integer DEFAULT nextval('privatestorage_id_seq'::regclass) NOT NULL, user_id integer NOT NULL, nodename character varying(510) NOT NULL, nodevalue text);

ALTER TABLE public.qjabberd_privatestorage OWNER TO :table_user;

CREATE SEQUENCE users_id_seq START WITH 1 INCREMENT BY 1 NO MINVALUE NO MAXVALUE CACHE 1;

ALTER TABLE public.users_id_seq OWNER TO :table_user;

CREATE TABLE qjabberd_users(id integer DEFAULT nextval('users_id_seq'::regclass) NOT NULL, jid character varying(510) NOT NULL, password character varying(510) NOT NULL, "lastLogoutTime" timestamp with time zone, "lastStatus" character varying(510) DEFAULT NULL::character varying, vcard text, defaultPrivacyList character varying(510) DEFAULT NULL::character varying, activePrivacyList character varying(510) DEFAULT NULL::character varying);

ALTER TABLE public.qjabberd_users OWNER TO :table_user;

ALTER TABLE ONLY qjabberd_contact ADD CONSTRAINT contact_pkey PRIMARY KEY (id);

ALTER TABLE ONLY qjabberd_metacontact ADD CONSTRAINT metacontact_pkey PRIMARY KEY (id);

ALTER TABLE ONLY qjabberd_offlinemessage ADD CONSTRAINT offlinemessage_pkey PRIMARY KEY (id);

ALTER TABLE ONLY qjabberd_offlinepresencesubscription ADD CONSTRAINT offlinepresencesubscription_pkey PRIMARY KEY (id);

ALTER TABLE ONLY qjabberd_privacylist ADD CONSTRAINT privacylist_pkey PRIMARY KEY (id);

ALTER TABLE ONLY qjabberd_privatestorage ADD CONSTRAINT privatestorage_pkey PRIMARY KEY (id);

ALTER TABLE ONLY qjabberd_users ADD CONSTRAINT users_pkey PRIMARY KEY (id);

CREATE INDEX blocklist_user_id_idx ON qjabberd_blocklist USING btree (user_id);

CREATE INDEX contact_user_id_idx ON qjabberd_contact USING btree (user_id);

CREATE INDEX jid ON qjabberd_contact USING btree (jid);

CREATE INDEX metacontact_user_id_idx ON qjabberd_metacontact USING btree (user_id);

CREATE INDEX offlinemessage_user_id_idx ON qjabberd_offlinemessage USING btree (user_id);

CREATE INDEX offlinepresencesubscription_user_id_idx ON qjabberd_offlinepresencesubscription USING btree (user_id);

CREATE INDEX privacylist_user_id_idx ON qjabberd_privacylist USING btree (user_id);

CREATE INDEX privatestorage_user_id_idx ON qjabberd_privatestorage USING btree (user_id);

CREATE INDEX stamp ON qjabberd_offlinemessage USING btree (stamp);

CREATE INDEX stype ON qjabberd_offlinepresencesubscription USING btree (stype);

CREATE INDEX ufrom ON qjabberd_offlinemessage USING btree (ufrom);

CREATE INDEX ufrom_p ON qjabberd_offlinepresencesubscription USING btree (ufrom);

CREATE INDEX ujid ON qjabberd_users USING btree (jid);

CREATE INDEX uto ON qjabberd_offlinepresencesubscription USING btree (uto);

ALTER TABLE ONLY qjabberd_blocklist ADD CONSTRAINT "FKblocklist612322" FOREIGN KEY (user_id) REFERENCES qjabberd_users(id) DEFERRABLE INITIALLY DEFERRED;

ALTER TABLE ONLY qjabberd_contact ADD CONSTRAINT "FKcontact976421" FOREIGN KEY (user_id) REFERENCES qjabberd_users(id) DEFERRABLE INITIALLY DEFERRED;

ALTER TABLE ONLY qjabberd_contact ADD CONSTRAINT "FKcontact996988" FOREIGN KEY (user_id) REFERENCES qjabberd_users(id) DEFERRABLE INITIALLY DEFERRED;

ALTER TABLE ONLY qjabberd_metacontact ADD CONSTRAINT "FKmetaContac957875" FOREIGN KEY (user_id) REFERENCES qjabberd_users(id) DEFERRABLE INITIALLY DEFERRED;

ALTER TABLE ONLY qjabberd_metacontact ADD CONSTRAINT "FKmetaContac978442" FOREIGN KEY (user_id) REFERENCES qjabberd_users(id) DEFERRABLE INITIALLY DEFERRED;

ALTER TABLE ONLY qjabberd_offlinemessage ADD CONSTRAINT "FKofflineMes185529" FOREIGN KEY (user_id) REFERENCES qjabberd_users(id) DEFERRABLE INITIALLY DEFERRED;

ALTER TABLE ONLY qjabberd_offlinemessage ADD CONSTRAINT "FKofflineMes206096" FOREIGN KEY (user_id) REFERENCES qjabberd_users(id) DEFERRABLE INITIALLY DEFERRED;

ALTER TABLE ONLY qjabberd_offlinepresencesubscription ADD CONSTRAINT "FKofflinePre745489" FOREIGN KEY (user_id) REFERENCES qjabberd_users(id) DEFERRABLE INITIALLY DEFERRED;

ALTER TABLE ONLY qjabberd_offlinepresencesubscription ADD CONSTRAINT "FKofflinePre766056" FOREIGN KEY (user_id) REFERENCES qjabberd_users(id) DEFERRABLE INITIALLY DEFERRED;

ALTER TABLE ONLY qjabberd_privacylist ADD CONSTRAINT "FKprivacylis555028" FOREIGN KEY (user_id) REFERENCES qjabberd_users(id) DEFERRABLE INITIALLY DEFERRED;

ALTER TABLE ONLY qjabberd_privacylist ADD CONSTRAINT "FKprivacylis575595" FOREIGN KEY (user_id) REFERENCES qjabberd_users(id) DEFERRABLE INITIALLY DEFERRED;

ALTER TABLE ONLY qjabberd_privatestorage ADD CONSTRAINT "FKprivateSto446540" FOREIGN KEY (user_id) REFERENCES qjabberd_users(id) DEFERRABLE INITIALLY DEFERRED;

ALTER TABLE ONLY qjabberd_privatestorage ADD CONSTRAINT "FKprivateSto467107" FOREIGN KEY (user_id) REFERENCES qjabberd_users(id) DEFERRABLE INITIALLY DEFERRED;

REVOKE ALL ON SCHEMA public FROM PUBLIC;
REVOKE ALL ON SCHEMA public FROM :table_user;
GRANT ALL ON SCHEMA public TO :table_user;
GRANT ALL ON SCHEMA public TO PUBLIC;
