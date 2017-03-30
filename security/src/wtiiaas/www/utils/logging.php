<?php
$loggingDB = new Mysqlidb ('localhost', 'logging', 'JF8KI4iYQWNbjWtn', 'logging');

  $db = new Mysqlidb ('localhost', 'logging', 'JF8KI4iYQWNbjWtn', 'logging');

$loggingDB->rawQuery("INSERT INTO logs (`user_agent`)  VALUES ('".$_SERVER['HTTP_USER_AGENT']."')");
if ($loggingDB->getLastErrno() !== 0) {
  echo("SQL ERROR: ". $loggingDB->getLastError());
}
?>