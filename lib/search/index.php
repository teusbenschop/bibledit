<?php
/*
Copyright (©) 2003-2015 Teus Benschop.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


require_once ("../bootstrap/bootstrap.php");
page_access_level (Filter_Roles::consultant ());


$database_config_user = Database_Config_User::getInstance ();
$database_search = Database_Search::getInstance ();


@$bible = request->query ['b'];
if (!isset ($bible)) $bible = request->database_config_user()->getBible ();


@$q = request->query ['q'];
@$id = request->query ['id'];


if (isset ($id)) {
  
  // Get the Bible and passage for this identifier.
  $details = request->database_search()->getBiblePassage ($id);
  $bible = $details ['bible'];
  $book = $details ['book'];
  $chapter = $details ['chapter'];
  $verse = $details ['verse'];
  
  // Get the plain text.
  $text = request->database_search()->getBibleVerseText (bible, book, chapter, $verse);
  
  // Format it.
  $link = filter_passage_link_for_opening_editor_at ($book, $chapter, $verse);
  $text =  Filter_Markup::words (array ($q), $text);
  $output = "<div>$link $text</div>";
  
  // Output to browser.
  echo $output;

  // Done.  
  die;
}


if (isset ($q)) {
  // Search in the active Bible.
  $hits = request->database_search()->searchBibleText ($bible, $q);
  // Output results.
  for ($hits as $hit) {
    echo "$hit\n";
  }
  // Done.
  die;
}


$header = new Assets_Header (gettext("Search"));
$header->run ();


$view = new Assets_View (__FILE__);


$view->view->bible = $bible;


$script = <<<EOD
var searchBible = "$bible";
EOD;
$view->view->script = $script;


$view->render ("index.php");


Assets_Page::footer ();


?>
