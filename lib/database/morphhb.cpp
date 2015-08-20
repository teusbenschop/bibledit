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


#include <database/morphhb.h>
#include <filter/url.h>
#include <filter/string.h>
#include <config/globals.h>
#include <database/sqlite.h>


// This is the database for the Hebrew Bible text plus limited parsings.
// Resilience: It is never written to.
// Chances of corruption are nearly zero.


sqlite3 * Database_MorphHb::connect ()
{
  return database_sqlite_connect ("morphhb");
}


// Get Hebrew words for $book $chapter $verse.
vector <string> Database_MorphHb::getVerse (int book, int chapter, int verse)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT word FROM morphhb WHERE book =");
  sql.add (book);
  sql.add ("AND chapter =");
  sql.add (chapter);
  sql.add ("AND verse =");
  sql.add (verse);
  sql.add (";");
  sqlite3 * db = connect ();
  vector <string> words = database_sqlite_query (db, sql.sql) ["word"];
  database_sqlite_disconnect (db);
  return words;
}


// Get array of book / chapter / verse of all passages that contain a $hebrew word.
vector <Passage> Database_MorphHb::searchHebrew (string hebrew)
{
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT DISTINCT book, chapter, verse FROM morphhb WHERE word =");
  sql.add (hebrew);
  sql.add (";");
  vector <Passage> hits;
  sqlite3 * db = connect ();
  map <string, vector <string> > result = database_sqlite_query (db, sql.sql);
  database_sqlite_disconnect (db);
  vector <string> books = result ["book"];
  vector <string> chapters = result ["chapter"];
  vector <string> verses = result ["verse"];
  for (unsigned int i = 0; i < books.size (); i++) {
    Passage passage = Passage ();
    passage.book = convert_to_int (books [i]);
    passage.chapter = convert_to_int (chapters [i]);
    passage.verse = verses [i];
    hits.push_back (passage);
  }
  return hits;
}


// Get text and parsing.
vector <Database_MorphHb_Item> Database_MorphHb::get (int book, int chapter, int verse)
{
  vector <Database_MorphHb_Item> items;
  SqliteSQL sql = SqliteSQL ();
  sql.add ("SELECT word, lemma FROM");
  sql.add ("morphhb");
  sql.add ("WHERE book =");
  sql.add (book);
  sql.add ("AND chapter =");
  sql.add (chapter);
  sql.add ("AND verse =");
  sql.add (verse);
  sql.add ("ORDER BY rowid;");
  sqlite3 * db = database_sqlite_connect ("morphhb");
  map <string, vector <string> > results = database_sqlite_query (db, sql.sql);
  database_sqlite_disconnect (db);
  vector <string> words = results ["word"];
  vector <string> lemmas = results ["lemma"];
  for (size_t i = 0; i < words.size (); i++) {
    Database_MorphHb_Item item;
    item.word = words[i];
    item.parsing = lemmas[i]; // The parsing is wrongly named 'lemma' in the database.
    items.push_back (item);
  }
  return items;
}
