<?php
/*
Copyright (©) 2003-2013 Teus Benschop.

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
?>
<h1><?php echo gettext ("Changes") ?></h1>
<p><?php echo gettext ("Bibledit-Web tracks the changes in the text of the Bibles.") ?></p>
<p>
  <?php echo gettext ("If there have been changes, it will generate lists of them at midnight.") ?> 
  <a href="changes.php?generate="><?php echo gettext ("Generate fresh lists now.") ?></a>
</p>
<p class="error"><?php echo $this->error ?></p>
<p class="success"><?php echo $this->success ?></p>
<p><a href="../downloads/changes"><?php echo gettext ("View the lists in the downloads section.") ?></a></p>
<br>
<p>
  <?php echo gettext ("Bibledit-Web can email lists with the changes to the users.") ?>
  <?php echo gettext ("It can also generate Change Notifications for users who have enabled that option.") ?>
  <a href="../user/notifications.php"><?php echo gettext ("Settings are in the notifications.") ?></a>
</p>
<br>
<p><?php echo gettext ("List of users who still have change notifications awaiting their approval:") ?></p>
<table>
<?php foreach ($this->users as $offset => $user) { ?>
  <tr>
    <td><?php echo $this->users[$offset] ?></td>
    <td>|</td>
    <td><?php echo $this->count[$offset] ?></td>
    <td>|</td>
    <td><a href="changes.php?clear=<?php echo $this->users[$offset] ?>">[<?php echo gettext ("clear") ?>]</a></td>
  </tr>
<?php } ?>
</table>
