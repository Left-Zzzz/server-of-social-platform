/*
Navicat MySQL Data Transfer

Source Server         : localhost
Source Server Version : 50650
Source Host           : localhost:3306
Source Database       : ssp

Target Server Type    : MYSQL
Target Server Version : 50650
File Encoding         : 65001

Date: 2022-08-08 15:14:18
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for tb_message
-- ----------------------------
DROP TABLE IF EXISTS `tb_message`;
CREATE TABLE `tb_message` (
  `user_id` int(11) DEFAULT NULL,
  `message_id` int(11) NOT NULL DEFAULT '0',
  `content` varchar(10240) DEFAULT NULL,
  `publish_time` int(11) DEFAULT NULL,
  `last_modify_time` int(11) DEFAULT NULL,
  PRIMARY KEY (`message_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for tb_photo
-- ----------------------------
DROP TABLE IF EXISTS `tb_photo`;
CREATE TABLE `tb_photo` (
  `user_id` int(11) NOT NULL,
  `publisher_id` int(11) DEFAULT NULL,
  `publish_time` int(11) DEFAULT NULL,
  `publish_message_id` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------
-- Table structure for tb_relation_black
-- ----------------------------
DROP TABLE IF EXISTS `tb_relation_black`;
CREATE TABLE `tb_relation_black` (
  `user_id` int(11) NOT NULL DEFAULT '0',
  `other_id` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`,`other_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------
-- Table structure for tb_relation_friend
-- ----------------------------
DROP TABLE IF EXISTS `tb_relation_friend`;
CREATE TABLE `tb_relation_friend` (
  `user_id` int(11) NOT NULL DEFAULT '0',
  `other_id` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`,`other_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------
-- Table structure for tb_user
-- ----------------------------
DROP TABLE IF EXISTS `tb_user`;
CREATE TABLE `tb_user` (
  `user_id` int(11) NOT NULL DEFAULT '0',
  `user_name` varchar(64) DEFAULT NULL,
  `password` varchar(64) DEFAULT NULL,
  `nick_name` varchar(64) DEFAULT NULL,
  `reg_time` int(11) DEFAULT NULL,
  `reg_from` int(11) DEFAULT NULL,
  `login_time` int(11) DEFAULT NULL,
  `logout_time` int(11) DEFAULT NULL,
  `fresh_time` int(11) DEFAULT NULL,
  PRIMARY KEY (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------
-- Table structure for tb_var
-- ----------------------------
DROP TABLE IF EXISTS `tb_var`;
CREATE TABLE `tb_var` (
  `id` int(11) DEFAULT NULL,
  `str` varchar(256) DEFAULT NULL,
  `value` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
