<?php

require_once ("../bootstrap/bootstrap.php");
page_access_level (TRANSLATOR_LEVEL);

$smarty = new Smarty_Bibledit (__FILE__);
$smarty->display("index.tpl");

?>
