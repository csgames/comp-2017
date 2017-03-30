<?php
error_reporting(E_ALL);
ini_set('display_errors', 1);

require_once '../utils/utils.php';

$query = file_get_contents('php://input');

//if (isset($_POST['query']) && !empty($_POST['query'])) {
if (isset($query) && !empty($query)) {

  $xml = simplexml_load_string($query, 'SimpleXMLElement', LIBXML_NOENT);

  // check if valid XML
  if ($xml === false) {
    foreach(libxml_get_errors() as $error) {
      echo $error->message;
    }
  }

  //authenticate
  $key_guid = $xml->identity->key;
  if (isset($key_guid)) {
    if (false === ($client_id = authenticate_api($key_guid->__toString()))) {
      die("<error>
  <identity>
    <key>".$key_guid."</key>
  </identity>
  <message>Invalid key. You either made a typo or you forgot to PAY YOUR BILLS!</message>
</error>");
    }
  } else {
    //not authenticated
    die("<error>Could not authenticate the request</error>");
  }

  $action = $xml->query;
  if (isset($action)) {
    switch (xml_attribute($action,"action")) {
      case 'getTime':
        echo "<response>".exec("date")."</response>";
        break;

      case 'getTimeWithFormat':
        // demo user      
        if ($client_id === 1) {
          echo "<error>This is a premium feature! If only there was a way to obtain a premium API key...</error>";
          die();
        }
        $format = xml_attribute($action,"format");
        echo "<response>".exec('/bin/sh -c \'date +"'.$format.'"\' 2>&1')."</response>";
        break;

      case 'help':
        echo "<response>
  <query action=\"getTime\" type=\"free\">
    <description>Get the current time in GMT<description>
  </query>

  <query action=\"getTimeWithFormat\" type=\"premium\">
    <description>Get the time with a custom format. Simply add a `format' attribute to your query and it will be appended after the `date' GNU command.<description>
    <example>
    <![CDATA[
      <query action=\"getTimeWithFormat\" format=\"%m-%d-%y\" />
    ]]>
  </example>
  </query>

  <query action=\"\"></query>
</response>";
        break;
      
      default:
        echo "<error>This action is unknown. Use action `help' for more options</error>";
    }
  }



} else {
    echo "<error>Unexpected answer</error>";
  }?>