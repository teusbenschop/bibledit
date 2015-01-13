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
page_access_level (Filter_Roles::translator ());


// Get search variables from the query.
@$bible = request->query ['b'];
@$searchfor = request->query ['q'];
@$casesensitive = (request->query ['c'] == "true");


$database_search = Database_Search::getInstance ();


// Do the search.
if ($casesensitive) {
  $hits = request->database_search()->searchBibleTextCaseSensitive ($bible, $searchfor);
} else {
  $hits = request->database_search()->searchBibleText ($bible, $searchfor);
}


// Output identifiers of the search results.
for ($hits as $hit) {
  echo "$hit\n";
}


?>
