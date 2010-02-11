<?php
/**
* @package bibledit
*/
/*
 ** Copyright (©) 2003-2009 Teus Benschop.
 **
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation; either version 3 of the License, or
 ** (at your option) any later version.
 **  
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **  
 ** You should have received a copy of the GNU General Public License
 ** along with this program; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **  
 */


/**
* This is a database in memory for use by the Poor Man's Crontab.
* Memory is chosen since the information in it is volatile, which is what is needed.
*/
class Database_Cron
{
  /**
  * Singleton logic.
  */
  private static $instance;
  private function __construct() {
    $database_instance = Database_Instance::getInstance();
    $query = "CREATE TABLE IF NOT EXISTS cron (flag int) ENGINE = MEMORY;";
    $database_instance->mysqli->query ($query);
  } 
  public static function getInstance() 
  {
    if ( empty( self::$instance ) ) {
      self::$instance = new Database_Cron();
    }
    return self::$instance;
  }

  public function setFlag () {
    $server = Database_Instance::getInstance ();
    $pid = getmypid();
    $server->mysqli->query ("INSERT INTO cron VALUES ($pid);");
  }
  
  public function clearFlag () {
    $server = Database_Instance::getInstance ();
    $server->mysqli->query ("DELETE FROM cron;");
  }
  
  public function getFlag () {
    $server = Database_Instance::getInstance ();
    $result = $server->mysqli->query ("SELECT * FROM cron;");
    return ($result->num_rows > 0);
  }

}



?>
