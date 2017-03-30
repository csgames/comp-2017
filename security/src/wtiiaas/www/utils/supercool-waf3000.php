<?php

if (isset($_SERVER["HTTP_USER_AGENT"]) && strpos($_SERVER['HTTP_USER_AGENT'],"sqlmap") !== false) {
  die("You are being naughty");
  
}?>