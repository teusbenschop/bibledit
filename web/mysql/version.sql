USE `BIBLEDITDATABASE`;

CREATE TABLE IF NOT EXISTS version (
  id int auto_increment primary key,
  name varchar (256),
  version int
) engine = MyISAM;

DROP PROCEDURE IF EXISTS upgrades;
DELIMITER ;;
CREATE PROCEDURE upgrades ()
BEGIN
  DECLARE CONTINUE HANDLER FOR 1060 BEGIN END;
  DECLARE CONTINUE HANDLER FOR 1061 BEGIN END;
  DECLARE CONTINUE HANDLER FOR 1091 BEGIN END;
END;;
CALL upgrades();;
DROP PROCEDURE upgrades;
