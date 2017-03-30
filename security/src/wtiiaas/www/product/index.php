<?php
include_once '../utils/utils.php';

$api_db = new Mysqlidb ('localhost', 'api', 'e9Cp0URepROiCckO', 'api');
$api_db->where('client_id',1); // demo client

$demo_guid = $api_db->getValue('apikeys','key_guid');
?>
<!DOCTYPE html>
<html lang="en">

  <head>

    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="description" content="">
    <meta name="author" content="">

    <title>What Time Is It As A Service - Time, at your service!</title>

    <!-- Bootstrap Core CSS -->
    <link href="/css/bootstrap.min.css" rel="stylesheet">

    <!-- Custom CSS -->
    <link href="/css/landing-page.css" rel="stylesheet">

    <!-- Custom Fonts -->
    <link href="/font-awesome/css/font-awesome.min.css" rel="stylesheet" type="text/css">
    <link href="https://fonts.googleapis.com/css?family=Lato:300,400,700,300italic,400italic,700italic" rel="stylesheet" type="text/css">

    <!-- HTML5 Shim and Respond.js IE8 support of HTML5 elements and media queries -->
    <!-- WARNING: Respond.js doesn't work if you view the page via file:// -->
    <!--[if lt IE 9]>
    <script src="https://oss.maxcdn.com/libs/html5shiv/3.7.0/html5shiv.js"></script>
    <script src="https://oss.maxcdn.com/libs/respond.js/1.4.2/respond.min.js"></script>
    <![endif]-->
    <style>


    #query {
      width: 48%;
      float: left;
    }
    #response {
      width: 48%;
      float: right;
    }
    textarea {
      font-family:Consolas,Monaco,Lucida Console,Liberation Mono,DejaVu Sans Mono,Bitstream Vera Sans Mono,Courier New, monospace;
    }

    </style>

    <!-- jQuery -->
    <script src="/js/jquery.js"></script>

    <!-- Bootstrap Core JavaScript -->
    <script src="/js/bootstrap.min.js"></script>    

    <script>
    $( document ).ready(function() {
      $("#query-button").click(function (){
        $.post("/api/api.php", $("#query-value").val())
           .done(function(data) {
             $("#response-value").val(data);
           });
      })
    });
    </script>
    
  </head>

  <body>

    <!-- Navigation -->
    <nav class="navbar navbar-default navbar-fixed-top topnav" role="navigation">
      <div class="container topnav">
        <!-- Brand and toggle get grouped for better mobile display -->
        <div class="navbar-header">
          <button type="button" class="navbar-toggle" data-toggle="collapse" data-target="#bs-example-navbar-collapse-1">
            <span class="sr-only">Toggle navigation</span>
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
          </button>
          <a class="navbar-brand topnav" href="/intranet/">Intranet</a>
        </div>
        <!-- Collect the nav links, forms, and other content for toggling -->
        <div class="collapse navbar-collapse" id="bs-example-navbar-collapse-1">
          <ul class="nav navbar-nav navbar-right">
            <li>
              <a href="/">Home</a>
            </li>
          </ul>
        </div>
        <!-- /.navbar-collapse -->
      </div>
      <!-- /.container -->
    </nav>



    <!-- Page Content -->
    <div style="padding-top:60px;" class="container">
      <div class="row">
        <div class="col-lg-12">
          <h1 class="page-header">Try the product first <small>We won't even bill you!</small></h1>
        </div>
        <center>
          <table>
            <table class="table table-striped">
              <thead>
                <tr>
                  <th>Block</th>
                  <th>Plan $</th>
                  <th><code>action</code> attribute</th>
                  <th>Other attributes</th>
                  <th>Description</th>
                </tr>
              </thead>
              <tbody>
                <tr>
                  <td><code>identity</code></td>
                  <td>-</td>
                  <td>-</td>
                  <td>-</td>
                  <td>API key goes inside a <code>&lt;key&gt;&lt;/key&gt;</code> block</td>
                </tr>
                <tr>
                  <td><code>query</code></td>
                  <td>-</td>
                  <td><code>help</code></td>
                  <td>-</td>
                  <td>Shows some samples of queries our API can understand</td>
                </tr>
                <tr>
                  <td><code>query</code></td>
                  <td>Free</td>
                  <td><code>getTime</code></td>
                  <td>-</td>
                  <td>Returns the current time</td>
                </tr>
                <tr>
                  <td><code>query</code></td>
                  <td>Premium</td>
                  <td><code>getTimeWithFormat</code></td>
                  <td><code>format="%m-%d-%y"</code></td>
                  <td>Get the time with a custom format</td>
                </tr>
              </tbody>
            </table>
        </center>
      </div>
      <div id="query">
        
        <textarea id="query-value" rows="10" style="width:100%">
<?xml version="1.0"?>
<WTIIaaS>
  <identity>
    <key><?= $demo_guid; ?></key>
  </identity>
  
  <query action="getTime">
  </query>
</WTIIaaS>
        </textarea>
      </div>
      <div id="response">
        <textarea id="response-value" rows="10" style="width:100%" readonly>
        </textarea>
      </div>

      <center><button id="query-button" type=button>Query API</button></center>
      <br />
    </div>

    <!-- Footer -->
    <footer>
      <div class="container">
        <div class="row">
          <div class="col-lg-12">
            <ul class="list-inline">
              <li>
                <a href="#">Home</a>
              </li>
              <li class="footer-menu-divider">&sdot;</li>
              <li>
                <a href="#about">About</a>
              </li>
              <li class="footer-menu-divider">&sdot;</li>
              <li>
                <a href="#services">Services</a>
              </li>
              <li class="footer-menu-divider">&sdot;</li>
              <li>
                <a href="#contact">Contact</a>
              </li>
            </ul>
            <p class="copyright text-muted small">Copyright &copy; Your Company 2014. All Rights Reserved</p>
          </div>
        </div>
      </div>
    </footer>

  </body>

</html>