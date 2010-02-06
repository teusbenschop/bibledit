<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <meta http-equiv="content-type" content="text/html; charset=UTF-8">
    </meta>
    <title>{t}Mail{/t}</title>
    <link rel="stylesheet" type="text/css" href="../css/stylesheet.css">
    </link>
  </head>
  <body>
    {include file=../assets/header_full.tpl} 
    <h1>{t}Mail{/t}</h1>
    <p>{t}This page deals with setting up the email system.{/t}</p>

    <h2>{t}Site name and email address{/t}</h2>
    <form action="mail" name="email" method="post">
      <p>{t}The name and the email address of the site will appear on any emails sent out from this site. If the Ndebele Bible, for example, is being translated on this site, then the name could be, e.g. "Ndebele Bible Translation", and the email address could be, e.g. "ndebeletranslation@gmail.com". Or anything that suits you. To switch the mailer off, make the fields empty.{/t}</p>
      <h4>{t}Enter the name and the email address, and press Submit.{/t}</h4>
      <table>
        <tr>
          <td align="right">{t}Name{/t}</td>
          <td><input type="text" name="sitename" maxlength="50" value="{$sitename}" /></td>
        </tr>
        <tr>
          <td align="right">{t}Email address{/t}</td>
          <td><input type="text" name="sitemail" maxlength="50" value="{$sitemail}" /></td>
        </tr>
        <tr>
          <td></td>
          <td><input type="submit" name="email" value={t}Submit{/t} /></td>
          <td class="error">{$site_name_error}</td>
          <td class="success">{$site_name_success}</td>
        </tr>
      </table>
    </form>

    <h2>{t}Retrieving mail{/t}</h2>
    <form action="mail" name="retrieve" method="post">
      <p>{t}Bibledit will retrieve email from the account specified below, and act on it. When you submit new values, Bibledit will attempt to contact this account and tell you how many messages there are. If something goes wrong, the message given may be cryptic at times. Please enter the correct details for this to work.{/t}</p>
      <h4>{t}Enter the account details, and press Submit.{/t}</h4>
      <table>
        <tr>
          <td align="right">{t}POP3 Host{/t}</td>
          <td><input type="text" name="storagehost" maxlength="50" value="{$storagehost}" /></td>
        </tr>
        <tr>
          <td align="right">{t}Username{/t}</td>
          <td><input type="text" name="storageusername" maxlength="50" value="{$storageusername}" /></td>
        </tr>
        <tr>
          <td align="right">{t}Password{/t}</td>
          <td><input type="text" name="storagepassword" maxlength="50" value="{$storagepassword}" /></td>
        </tr>
        <tr>
          <td></td>
          <td><input type="submit" name="retrieve" value={t}Submit{/t} onClick="this.value = '{t}Please wait{/t}'; return true;" /></td>
          <td class="error">{$storage_error}</td>
          <td class="success">{$storage_success}</td>
        </tr>
      </table>
    </form>

    <h2>{t}Sending mail{/t}</h2>
    <form action="mail" name="send" method="post">
      <p>{t}Bibledit will send out emails through the account specified below. When you submit new values, Bibledit will attempt to send an email the site's email address. If something goes wrong, the message given may be cryptic at times. Please enter the correct details for this to work. If the SMTP server does not need authenticaton, just leave the fields for the username and the password empty. If the SMTP Host is left empty, it will use the default sendmail transport.{/t}</p>
      <h4>{t}Enter the account details, and press Submit.{/t}</h4>
      <table>
        <tr>
          <td align="right">{t}SMTP Host{/t}</td>
          <td><input type="text" name="sendhost" maxlength="50" value="{$sendhost}" /></td>
        </tr>
        {* STMP username and password not yet implemented.  
        <tr>
          <td align="right">{t}Username{/t}</td>
          <td><input type="text" name="sendusername" maxlength="50" value="{$sendusername}" /></td>
        </tr>
        <tr>
          <td align="right">{t}Password{/t}</td>
          <td><input type="text" name="sendpassword" maxlength="50" value="{$sendpassword}" /></td>
        </tr>
        *}
        <tr>
          <td></td>
          <td><input type="submit" name="send" value={t}Submit{/t} onClick="this.value = '{t}Please wait{/t}'; return true;" /></td>
          <td class="error">{$send_error}</td>
          <td class="success">{$send_success}</td>
        </tr>
      </table>
    </form>


    {include file=../assets/footer_full.tpl} 
  </body>
</html>
