CREATE TABLE IF NOT EXISTS qjabberd_blocklist(user_id int(10) NOT NULL, jid varchar(255) COLLATE utf8_unicode_ci NOT NULL, KEY jid (jid)) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS qjabberd_contact(id int(10) NOT NULL AUTO_INCREMENT, user_id int(10) NOT NULL, approved int(1) DEFAULT NULL,ask varchar(9) COLLATE utf8_unicode_ci DEFAULT NULL, groups text COLLATE utf8_unicode_ci, jid varchar(255) COLLATE utf8_unicode_ci NOT NULL, name varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL, subscription varchar(4) COLLATE utf8_unicode_ci DEFAULT NULL, version varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL, PRIMARY KEY (id), KEY jid (jid), KEY user_id(user_id)) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

CREATE TABLE IF NOT EXISTS qjabberd_metacontact(id int(10) NOT NULL AUTO_INCREMENT, user_id int(10) NOT NULL, jid varchar(255) COLLATE utf8_unicode_ci NOT NULL, tag varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL, morder int(10) DEFAULT NULL, PRIMARY KEY (id), KEY user_id(user_id)) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

CREATE TABLE IF NOT EXISTS qjabberd_offlinemessage(id int(10) NOT NULL AUTO_INCREMENT, user_id int(10) NOT NULL, ufrom varchar(255) COLLATE utf8_unicode_ci NOT NULL, stamp varchar(255) COLLATE utf8_unicode_ci NOT NULL, otype varchar(255) COLLATE utf8_unicode_ci NOT NULL, obody text COLLATE utf8_unicode_ci NOT NULL, PRIMARY KEY (id), KEY user_id(user_id), KEY stamp (stamp), KEY ufrom (ufrom)) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

CREATE TABLE IF NOT EXISTS qjabberd_offlinepresencesubscription(id int(10) NOT NULL AUTO_INCREMENT, user_id int(10) NOT NULL, stype varchar(12) COLLATE utf8_unicode_ci NOT NULL, ufrom varchar(255) COLLATE utf8_unicode_ci NOT NULL, uto varchar(255) COLLATE utf8_unicode_ci NOT NULL, presenceStanza text COLLATE utf8_unicode_ci NOT NULL, PRIMARY KEY (id), KEY user_id(user_id), KEY ufrom (ufrom), KEY uto (uto), KEY stype (stype)) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

CREATE TABLE IF NOT EXISTS qjabberd_privacylist(id int(10) NOT NULL AUTO_INCREMENT, user_id int(10) NOT NULL, ptype varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL, pvalue varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL, action varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL, iorder int(11) DEFAULT NULL, child text COLLATE utf8_unicode_ci, privacyListName varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL, PRIMARY KEY (id), KEY user_id(user_id)) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

CREATE TABLE IF NOT EXISTS qjabberd_privatestorage(id int(10) NOT NULL AUTO_INCREMENT, user_id int(10) NOT NULL, nodename varchar(255) COLLATE utf8_unicode_ci NOT NULL, nodevalue text COLLATE utf8_unicode_ci, PRIMARY KEY (id), KEY user_id(user_id)) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

CREATE TABLE IF NOT EXISTS qjabberd_users(id int(10) NOT NULL AUTO_INCREMENT, jid varchar(255) COLLATE utf8_unicode_ci NOT NULL, password varchar(255) COLLATE utf8_unicode_ci NOT NULL, lastLogoutTime datetime DEFAULT NULL, lastStatus varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL, vcard text COLLATE utf8_unicode_ci, defaultPrivacyList varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL, activePrivacyList varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL, PRIMARY KEY (id), KEY jid (jid)) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

ALTER TABLE qjabberd_blocklist ADD CONSTRAINT FKblocklist612322 FOREIGN KEY (user_id) REFERENCES qjabberd_users (id);

ALTER TABLE qjabberd_contact ADD CONSTRAINT FKcontact976421 FOREIGN KEY (user_id) REFERENCES qjabberd_users (id), ADD CONSTRAINT FKcontact996988 FOREIGN KEY (user_id) REFERENCES qjabberd_users (id);

ALTER TABLE qjabberd_metacontact ADD CONSTRAINT FKmetaContac957875 FOREIGN KEY (user_id) REFERENCES qjabberd_users (id), ADD CONSTRAINT FKmetaContac978442 FOREIGN KEY (user_id) REFERENCES qjabberd_users (id);

ALTER TABLE qjabberd_offlinemessage ADD CONSTRAINT FKofflineMes185529 FOREIGN KEY (user_id) REFERENCES qjabberd_users (id), ADD CONSTRAINT FKofflineMes206096 FOREIGN KEY (user_id) REFERENCES qjabberd_users (id);

ALTER TABLE qjabberd_offlinepresencesubscription ADD CONSTRAINT FKofflinePre745489 FOREIGN KEY (user_id) REFERENCES qjabberd_users (id), ADD CONSTRAINT FKofflinePre766056 FOREIGN KEY (user_id) REFERENCES qjabberd_users (id);

ALTER TABLE qjabberd_privacylist ADD CONSTRAINT FKprivacylis555028 FOREIGN KEY (user_id) REFERENCES qjabberd_users (id), ADD CONSTRAINT FKprivacylis575595 FOREIGN KEY (user_id) REFERENCES qjabberd_users (id);

ALTER TABLE qjabberd_privatestorage ADD CONSTRAINT FKprivateSto446540 FOREIGN KEY (user_id) REFERENCES qjabberd_users (id), ADD CONSTRAINT FKprivateSto467107 FOREIGN KEY (user_id) REFERENCES qjabberd_users (id);
