<h1>{t}Help{/t}</h1>
<p><a href="introduction.php">{t}Introduction{/t}</a></p>
<p><a href="installation.php">{t}Installation{/t}</a></p>
<p><a href="about.php">{t}About{/t}</a></p>
{foreach key=key item=item from=$plugins} 
  <p><a href={$key}>{$item}</a></p>
{/foreach} 
