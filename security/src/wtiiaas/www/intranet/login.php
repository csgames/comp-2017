<?php
include_once '../utils/utils.php';
if (isset($_POST['employee_email']) && isset($_POST['employee_password'])) {
  $res = $db->rawQuery('SELECT * FROM employees WHERE employee_email = ? and employee_password = ?', array($_POST['employee_email'], md5($_POST['employee_password'])));
  if (sizeof($res) > 0) {
    $_SESSION['identity'] = $res[0];
    header('Location: /intranet/');
  } else {
        header('Location: /intranet/loginform.php?err=true');
  }
}

?>