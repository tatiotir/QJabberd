-- phpMyAdmin SQL Dump
-- version 4.0.4.1
-- http://www.phpmyadmin.net
--
-- Client: 127.0.0.1
-- Généré le: Ven 01 Août 2014 à 21:16
-- Version du serveur: 5.5.32
-- Version de PHP: 5.4.16

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Base de données: `qjabberd`
--
CREATE DATABASE IF NOT EXISTS `qjabberd` DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci;
USE `qjabberd`;

-- --------------------------------------------------------

--
-- Structure de la table `blocklist`
--

CREATE TABLE IF NOT EXISTS `blocklist` (
  `user_id` int(10) NOT NULL,
  `jid` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  KEY `jid` (`jid`),
  KEY `FKblocklist612322` (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Structure de la table `contact`
--

CREATE TABLE IF NOT EXISTS `contact` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `user_id` int(10) NOT NULL,
  `approved` int(1) DEFAULT NULL,
  `ask` varchar(9) COLLATE utf8_unicode_ci DEFAULT NULL,
  `groups` text COLLATE utf8_unicode_ci,
  `jid` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `name` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `subscription` varchar(4) COLLATE utf8_unicode_ci DEFAULT NULL,
  `version` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `FKcontact996988` (`user_id`),
  KEY `FKcontact976421` (`user_id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=5 ;

--
-- Structure de la table `metacontact`
--

CREATE TABLE IF NOT EXISTS `metacontact` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `user_id` int(10) NOT NULL,
  `jid` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `tag` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `order` int(10) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `FKmetaContac957875` (`user_id`),
  KEY `FKmetaContac978442` (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Structure de la table `offlinemessage`
--

CREATE TABLE IF NOT EXISTS `offlinemessage` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `user_id` int(10) NOT NULL,
  `ufrom` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `stamp` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `type` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `body` text COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id`),
  KEY `FKofflineMes206096` (`user_id`),
  KEY `FKofflineMes185529` (`user_id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=3 ;

--
-- Structure de la table `offlinepresencesubscription`
--

CREATE TABLE IF NOT EXISTS `offlinepresencesubscription` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `user_id` int(10) NOT NULL,
  `type` varchar(12) COLLATE utf8_unicode_ci NOT NULL,
  `ufrom` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `uto` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `presenceStanza` text COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id`),
  KEY `FKofflinePre745489` (`user_id`),
  KEY `FKofflinePre766056` (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Structure de la table `privacylist`
--

CREATE TABLE IF NOT EXISTS `privacylist` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `user_id` int(10) NOT NULL,
  `type` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `value` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `action` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `iorder` int(11) DEFAULT NULL,
  `child` text COLLATE utf8_unicode_ci,
  `privacylistname` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `FKprivacylis575595` (`user_id`),
  KEY `FKprivacylis555028` (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Structure de la table `privatestorage`
--

CREATE TABLE IF NOT EXISTS `privatestorage` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `user_id` int(10) NOT NULL,
  `nodename` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `nodevalue` text COLLATE utf8_unicode_ci,
  PRIMARY KEY (`id`),
  KEY `FKprivateSto467107` (`user_id`),
  KEY `FKprivateSto446540` (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Structure de la table `users`
--

CREATE TABLE IF NOT EXISTS `users` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `jid` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `password` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `lastLogoutTime` datetime DEFAULT NULL,
  `lastStatus` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `vcard` text COLLATE utf8_unicode_ci,
  `defaultprivacylist` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `activeprivacylist` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=3 ;

--
-- Contraintes pour la table `blocklist`
--
ALTER TABLE `blocklist`
  ADD CONSTRAINT `FKblocklist612322` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`);

--
-- Contraintes pour la table `contact`
--
ALTER TABLE `contact`
  ADD CONSTRAINT `FKcontact976421` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`),
  ADD CONSTRAINT `FKcontact996988` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`);

--
-- Contraintes pour la table `metacontact`
--
ALTER TABLE `metacontact`
  ADD CONSTRAINT `FKmetaContac957875` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`),
  ADD CONSTRAINT `FKmetaContac978442` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`);

--
-- Contraintes pour la table `offlinemessage`
--
ALTER TABLE `offlinemessage`
  ADD CONSTRAINT `FKofflineMes185529` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`),
  ADD CONSTRAINT `FKofflineMes206096` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`);

--
-- Contraintes pour la table `offlinepresencesubscription`
--
ALTER TABLE `offlinepresencesubscription`
  ADD CONSTRAINT `FKofflinePre745489` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`),
  ADD CONSTRAINT `FKofflinePre766056` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`);

--
-- Contraintes pour la table `privacylist`
--
ALTER TABLE `privacylist`
  ADD CONSTRAINT `FKprivacylis555028` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`),
  ADD CONSTRAINT `FKprivacylis575595` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`);

--
-- Contraintes pour la table `privatestorage`
--
ALTER TABLE `privatestorage`
  ADD CONSTRAINT `FKprivateSto446540` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`),
  ADD CONSTRAINT `FKprivateSto467107` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`);

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
