<?php
include_once '../utils/utils.php';
include_once '../utils/messaging.php';

confirm_auth();

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
  $post = true;
  //probably sending a message
  if (isset($_POST['message_to_employee_id']) && isset($_POST['message_subject']) && isset($_POST['message_content'])) {
    //content seems filled
    $insert_id = send_message($_POST['message_to_employee_id'], $_POST['message_subject'], $_POST['message_content']);
    $error = ($insert_id == null);
  }
}
?>
<!DOCTYPE html>
<html lang="en">
  <head>
    <title>WTIIAAS — Messaging</title>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css">
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.1.1/jquery.min.js"></script>
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js"></script>
    <style>
    /* Set height of the grid so .sidenav can be 100% (adjust if needed) */
    .row.content {height: 1500px}
    
    /* Set gray background color and 100% height */
    .sidenav {
      background-color: #f1f1f1;
      height: 100%;
    }
    
    /* Set black background color, white text and some padding */
    footer {
      background-color: #555;
      color: white;
      padding: 15px;
    }
    
    /* On small screens, set height to 'auto' for sidenav and grid */
    @media screen and (max-width: 767px) {
      .sidenav {
        height: auto;
        padding: 15px;
      }
      .row.content {height: auto;} 
    }
    </style>
  </head>
  <body>

    <div class="container-fluid">
        <div class="row content">
        <div class="col-sm-3 sidenav">
          <h4>WTIIAAS — Intranet</h4>
          <ul class="nav nav-pills nav-stacked">
            <li><a href="/intranet/">News</a></li>
            <li class="active"><a href="/messaging/">Messaging <span class="badge"><?= get_unread_message_count() ?></span></a></li>
            <li><a href="/intranet/logout.php">Log Out (<?=$_SESSION['identity']['employee_lname'].", ".$_SESSION['identity']['employee_fname']?>)</a></li>
          </ul><br>
          <div class="input-group">
            <input type="text" class="form-control" placeholder="Search Blog..">
            <span class="input-group-btn">
              <button class="btn btn-default" type="button">
                <span class="glyphicon glyphicon-search"></span>
              </button>
            </span>
          </div>
        </div>

        <form action="compose.php" method="POST">
          <div class="col-md-9">
            <h1>Messaging <?php
                              if (isset($post) && isset($error) && $error) { ?><span class="label label-danger">Error sending message</span></h1>
            <?php } elseif(isset($post) && isset($error) && !$error) {?><span class="label label-success">Message sent!</span></h1><?php } ?>
              <hr>
              <h3>Send message to:</h3>
              <div class="row">
                <div class="col-md-10">
                  <select name="message_to_employee_id">
                    <?php $employees = get_employee_list();
                    
                    foreach ($employees as $employee) { ?>
                    <option value="<?=$employee['employee_id']?>"><?=$employee['employee_lname'].", ".$employee['employee_fname']." (".$employee['role_name'].")"?></option>
                    <?php  
                    }
                    ?>
                  </select>
                </div>
              </div>

              <div class="row">
                <div class="col-md-2">
                  <h3>Subject:</h3>
                </div>
                <div class="col-md-10">
                  <h3><input name="message_subject" style="width:80%" type="text" /></h3>
                </div>
              </div>

              <div class="row">
                <div class="col-md-2">
                  <h3>Message:</h3>
                </div>
                <div class="col-md-10">
                  <h3><textarea name="message_content" rows="10" style="width:80%"></textarea></h3>
                </div>
              </div>
              <button type="submit">Send!</button>
          </div>
        </form>
      </div>
    </div>

    <footer class="container-fluid">
      <p>Footer Text</p>
    </footer>

  </body>
</html>
