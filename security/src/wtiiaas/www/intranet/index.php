<?php
include_once '../utils/utils.php';
include_once '../utils/messaging.php';
confirm_auth();
?>
<!DOCTYPE html>
<html lang="en">
  <head>
    <title>WTIIAAS — Intranet</title>
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
            <li class="active"><a href="#section1">News</a></li>
            <li><a href="/messaging/">Messaging <span class="badge"><?= get_unread_message_count() ?></span></a></li>
            <li><a href="/intranet/logout.php">Log Out</a></li>
          </ul><br>
          <div class="input-group">
            <input type="text" class="form-control" placeholder="Search Blog..">
            <span class="input-group-btn">
              <button class="btn btn-default" type="button" onclick="alert('We ran out of money to implement this.')">
                <span class="glyphicon glyphicon-search"></span>
              </button>
            </span>
          </div>
        </div>

        <div class="col-sm-9">
          <h4><small>RECENT POSTS</small></h4>
          
          <hr>
          <h2>Our new CEO!</h2>
          <h5><span class="glyphicon glyphicon-time"></span> Post by April Davis, Feb 22, 2017.</h5>
          <h5><span class="label label-success">Hiring</span></h5><br>
          <p>As you all know, Katerine J. Papp was forced to step down as CEO because of insider trading. Well, we've found a new CEO! I would like you all to meet Harden Jane, which will be our next CEO starting from right now.</p>
          <p>

            <p><span class="badge">1</span> Comments:</p><br>
            <div class="row">
              <div style="padding-left:100px;" class="col-sm-10">
                <h4>Harden Jane, <b>CEO</b>  <small>Feb 22, 2017</small></h4>
                <p>Thank you so much everyone for the warm welcome. As a new CEO, I want you all to know that I am always there to hear your you out. <span style="font-size:20px;font-weight:bold;">I promise that I will open and read every single message that you send me through the Intranet messaging system within <i><u>one minute</u></i> of it being received</span>.</p>
                <br>
              </div>
            </div>
            <hr>

            <h2>A word from our lovely CFO</h2>
            <h5><span class="glyphicon glyphicon-time"></span> Post by April Davis, Dec 24, 2016.</h5>
            <h5><span class="label label-danger">Profit</span> <span class="label label-primary">Overtime</span></h5><br>
            <p>Fellow employees, as you all know now, we are soon going to be going public. As expected, I will need you all to be working overtime for the foreseeable future. While I understand this measure might not feel pleasant, it is for the best of the company.</p>
            <p>In other news, the profits this month have been CRAZY! I am not sure why, perhaps it is because our executives have been working extra hard lately. Anyhow, I swear one day I will take a few of you lucky peeps out on my Yachts!</p>
            <img src="/img/yacht.jpg" />
            <br><br>
            
            <hr>

        </div>
      </div>
    </div>

    <footer class="container-fluid">
      <p>Footer Text</p>
    </footer>

  </body>
</html>
