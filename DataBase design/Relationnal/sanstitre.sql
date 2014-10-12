DROP INDEX jid ON users;
DROP INDEX ufrom ON offlinePresenceSubscription;
DROP INDEX uto ON offlinePresenceSubscription;
DROP INDEX ufrom ON offlineMessage;
DROP INDEX stamp ON offlineMessage;
CREATE TABLE blocklist (
  user_id int(10) NOT NULL, 
  jid     varchar(255) NOT NULL, 
  INDEX (jid));
ALTER TABLE contact ADD INDEX FKcontact976421 (user_id), ADD CONSTRAINT FKcontact976421 FOREIGN KEY (user_id) REFERENCES users (id);
ALTER TABLE metaContact ADD INDEX FKmetaContac978442 (user_id), ADD CONSTRAINT FKmetaContac978442 FOREIGN KEY (user_id) REFERENCES users (id);
ALTER TABLE privateStorage ADD INDEX FKprivateSto446540 (user_id), ADD CONSTRAINT FKprivateSto446540 FOREIGN KEY (user_id) REFERENCES users (id);
ALTER TABLE offlineMessage ADD INDEX FKofflineMes185529 (user_id), ADD CONSTRAINT FKofflineMes185529 FOREIGN KEY (user_id) REFERENCES users (id);
ALTER TABLE offlinePresenceSubscription ADD INDEX FKofflinePre766056 (user_id), ADD CONSTRAINT FKofflinePre766056 FOREIGN KEY (user_id) REFERENCES users (id);
ALTER TABLE privacylist ADD INDEX FKprivacylis555028 (user_id), ADD CONSTRAINT FKprivacylis555028 FOREIGN KEY (user_id) REFERENCES users (id);
ALTER TABLE blocklist ADD INDEX FKblocklist612322 (user_id), ADD CONSTRAINT FKblocklist612322 FOREIGN KEY (user_id) REFERENCES users (id);
