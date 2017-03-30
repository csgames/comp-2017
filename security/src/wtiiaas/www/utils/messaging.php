<?php

function get_unread_message_count() {
  $db = MysqliDb::getInstance();
  $db->where('message_to_employee_id', $_SESSION['identity']['employee_id']);
  $db->where('message_read', 0);
  return $db->getValue("messaging", "count(1)");
}

function get_messages() {
  $db = MysqliDb::getInstance();
  return $db->rawQuery('SELECT message_id, employee_fname, employee_lname, message_subject, message_read FROM messaging, employees WHERE message_to_employee_id = ? AND messaging.message_from_employee_id = employees.employee_id;', array($_SESSION['identity']['employee_id']));
}

function unread($message) {
  return $message['message_read'] === 0;
}
function read($message) {
  return $message['message_read'] !== 0;
}

function get_message_by_id() {
  $db = MysqliDb::getInstance();
  $message_content = $db->rawQuery('SELECT message_id, employee_fname, employee_lname, employee_email, message_subject, message_content, message_read FROM messaging, employees WHERE message_to_employee_id = ? AND message_id = ? AND messaging.message_from_employee_id = employees.employee_id;', array($_SESSION['identity']['employee_id'], $_GET['id']));

  if (sizeof($message_content) > 0) {
    // we found a record
    $message_content = $message_content[0];
  } else {
    return;
  }
  
  if ($message_content['message_read'] === 0) {
    // make it read
    $db->where('message_id', $message_content['message_id']);
    $db->update('messaging', array("message_read" => 1));
  }

  return $message_content;
}


function get_message_by_id_latest() {
  $db = MysqliDb::getInstance();
  $message_content = $db->rawQuery('SELECT message_id, employee_fname, employee_lname, employee_email, message_subject, message_content, message_read FROM messaging, employees WHERE message_to_employee_id = ? AND messaging.message_from_employee_id = employees.employee_id AND message_read = 0 ORDER BY message_id DESC LIMIT 0,1;', array($_SESSION['identity']['employee_id']));

  if (sizeof($message_content) > 0) {
    // we found a record
    $message_content = $message_content[0];
  } else {
    return;
  }
  
  if ($message_content['message_read'] === 0) {
    // make it read
    $db->where('message_id', $message_content['message_id']);
    $db->update('messaging', array("message_read" => 1));
  }

  return $message_content;
}


function get_employee_list () {
  $db = MysqliDb::getInstance();
  $employees = $db->rawQuery('SELECT employee_id, employee_fname, employee_lname, role_name FROM employees, roles WHERE employee_role = role_id');
  return $employees;
}

function send_message ($message_to_employee_id, $message_subject, $message_content) {
  $db = MysqliDb::getInstance();
  $data = array('message_to_employee_id' => $message_to_employee_id,
                'message_from_employee_id' => $_SESSION['identity']['employee_id'],
                'message_subject' => $message_subject,
                'message_content' => $message_content);
  $id = $db->insert('messaging', $data);
  return $id;
}

?>