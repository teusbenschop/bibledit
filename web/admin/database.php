<?php


require_once ("../bootstrap/bootstrap.php");
page_access_level (ADMIN_LEVEL);


$database_instance = Database_Instance::getInstance(true);
$smarty = new Smarty_Bibledit (__FILE__);


// Show number of tables.
$result = $database_instance->mysqli->query ("SHOW TABLES;");
$smarty->assign ("tables_count_before", $result->num_rows);


// The versions table.
$database_versions = Database_Versions::getInstance ();
$database_versions->verify ();


// The user table.
$database_users = Database_Users::getInstance();
$database_users->verify ();


// The general configuration table.
$database_config_general = Database_Config_General::getInstance ();
$database_config_general->verify ();


// The logs table.
$database_logs = Database_Logs::getInstance ();
$database_logs->verify();


// The mail table.
$database_mail = Database_Mail::getInstance ();
$database_mail->verify();


// Show number of tables again.
$result = $database_instance->mysqli->query ("SHOW TABLES;");
$smarty->assign ("tables_count_after", $result->num_rows);


// Display page.
$smarty->display("database.tpl");


?>
