-- phpMyAdmin SQL Dump
-- version 2.9.1.1-Debian-1
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Dec 27, 2006 at 03:18 PM
-- Server version: 5.0.30
-- PHP Version: 5.2.0-8
-- 
-- Database: `cvk`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `class`
-- 

CREATE TABLE `class` (
  `class_id` tinyint(2) unsigned NOT NULL auto_increment,
  `project_id` tinyint(2) NOT NULL,
  `class_name` varchar(32) NOT NULL,
  PRIMARY KEY  (`class_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=5 ;

-- 
-- Dumping data for table `class`
-- 

INSERT INTO `class` (`class_id`, `project_id`, `class_name`) VALUES 
(1, 1, 'FN1'),
(2, 1, 'FN2'),
(3, 2, 'CPCM3'),
(4, 2, 'CBGM2');

-- --------------------------------------------------------

-- 
-- Table structure for table `group`
-- 

CREATE TABLE `group` (
  `group_id` tinyint(2) NOT NULL auto_increment,
  `project_id` tinyint(2) NOT NULL,
  `group_name` varchar(2) NOT NULL,
  PRIMARY KEY  (`group_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=13 ;

-- 
-- Dumping data for table `group`
-- 

INSERT INTO `group` (`group_id`, `project_id`, `group_name`) VALUES 
(1, 1, 'J'),
(2, 1, 'K'),
(3, 1, 'L'),
(4, 1, 'M'),
(5, 1, 'N'),
(6, 1, 'O'),
(7, 2, 'A'),
(8, 2, 'B'),
(9, 2, 'L'),
(10, 2, 'M'),
(11, 2, 'N'),
(12, 2, 'O');

-- --------------------------------------------------------

-- 
-- Table structure for table `project`
-- 

CREATE TABLE `project` (
  `project_id` tinyint(2) unsigned NOT NULL auto_increment,
  `project_name` varchar(32) NOT NULL,
  PRIMARY KEY  (`project_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=3 ;

-- 
-- Dumping data for table `project`
-- 

INSERT INTO `project` (`project_id`, `project_name`) VALUES 
(1, 'TPC'),
(2, 'MSP');

-- --------------------------------------------------------

-- 
-- Table structure for table `video`
-- 

CREATE TABLE `video` (
  `video_id` int(10) unsigned NOT NULL auto_increment,
  `tape_date` date NOT NULL,
  `project_id` mediumint(8) unsigned NOT NULL,
  `class_id` mediumint(8) unsigned NOT NULL,
  `video_type` smallint(3) NOT NULL,
  `group_id` mediumint(8) unsigned NOT NULL,
  `session_number` smallint(5) unsigned NOT NULL,
  `position` enum('F','R','X') NOT NULL,
  `tape_number` tinyint(3) unsigned NOT NULL,
  `encode_time` datetime NOT NULL,
  `creation_time` datetime NOT NULL,
  PRIMARY KEY  (`video_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=2 ;

-- 
-- Dumping data for table `video`
-- 

INSERT INTO `video` (`video_id`, `tape_date`, `project_id`, `class_id`, `video_type`, `group_id`, `session_number`, `position`, `tape_number`, `encode_time`, `creation_time`) VALUES 
(1, '2004-01-01', 2, 3, 1, 7, 1, 'F', 1, '0000-00-00 00:00:00', '0000-00-00 00:00:00');
