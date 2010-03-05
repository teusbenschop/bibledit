<?php
require_once ("../bootstrap/bootstrap.php");
page_access_level (MANAGER_LEVEL);
$header = gettext ("Would you like to change the numbering of the note?");
$info_top = gettext ("Here are the various options:");
$info_bottom = gettext ("Please pick one.");
$sheet = $_GET['sheet'];
$style = $_GET['style'];
$dialog_list = new Dialog_List (array ("sheet"=> $sheet, "style" => $style), $header, $info_top, $info_bottom);
$styles_logic = Styles_Logic::getInstance();
for ($i = NoteNumbering123; $i <= NoteNumberingUser; $i++) {
  $dialog_list->add_row ($styles_logic->noteNumberingText ($i), "&userint1=$i");
}
$dialog_list->run ();
?>
