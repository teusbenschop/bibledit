<?php
/**
* @package bibledit
*/
/*
 ** Copyright (©) 2003-2013 Teus Benschop.
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

class Assets_Header
{
  private static $instance;
  private $view;
  private $includeJQuery = false;
  private $includeJQueryUI = false;

  public function __construct ($title) 
  {
    $this->view = new Assets_View (__FILE__);
    $this->view->view->title = $title;
  }

  public function jQueryOn ()
  {
    $this->includeJQuery = true;
  }
  
  public function jQueryUIOn ()
  {
    $this->jQueryOn ();
    $this->includeJQueryUI = true;
  }
  
  // Adds an 'onload' statement to the <body> html tag.
  // $code could be, e.g.: onload="document.form.user.focus();"
  public function setBodyOnload ($code)
  {
    $this->view->view->onLoadStatement = $code;
  }
  
  // Runs the header.
  public function run ()
  {
    $this->view->view->include_jquery = $this->includeJQuery;
    $this->view->view->include_jquery_ui = $this->includeJQueryUI;
    $this->view->render ("xhtml_start.php");
    $this->view->render ("header_full.php");
    
  }

}


?>
