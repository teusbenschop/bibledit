-- Copyright (©) 2003-2013 Teus Benschop.

-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 3 of the License, or
-- (at your option) any later version.

-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.

-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

USE `BIBLEDITDATABASE`;

CREATE TABLE IF NOT EXISTS users (
  username varchar(30) primary key,
  password varchar(32),
  id varchar(32),
  level tinyint(1) unsigned not null,
  email varchar(256),
  timestamp int(11) unsigned not null
) engine = MyISAM;


DROP PROCEDURE IF EXISTS upgrades;
DELIMITER ;;
CREATE PROCEDURE upgrades ()
BEGIN
  DECLARE CONTINUE HANDLER FOR 1060 BEGIN END;
  DECLARE CONTINUE HANDLER FOR 1061 BEGIN END;
  DECLARE CONTINUE HANDLER FOR 1091 BEGIN END;
  ALTER TABLE users ENGINE = MYISAM;
END;;
CALL upgrades();;
DROP PROCEDURE upgrades;
