var page = require('webpage').create();
var url = "http://127.0.0.1/messaging/read-latest-message.php";
var timeout = 2000;
var postData = 'employee_email=jane.harden@wtiiaas.csg&employee_password=SupeReLitePassWerd12--@@';

page.onNavigationRequested = function(url, type, willNavigate, main) {
    console.log("[URL] URL="+url);  
};
 
page.settings.resourceTimeout = timeout;
page.onResourceTimeout = function(e) {
    setTimeout(function(){
        console.log("[INFO] Timeout")
        phantom.exit();
    }, 1);
};
 

phantom.addCookie({
    'name': 'Flag',
    'value': 'CSG-WowItsAsIfXSSIsActuallyABigDeal',
    'domain': '127.0.0.1',
    'path': '/',
    'httponly': false
});

console.log("[INFO] Querying ");

page.open(url, 'POST', postData, function(status) {
    console.log("[INFO] rendered page");
    setTimeout(function(){
        phantom.exit();
    }, 7000);
});
