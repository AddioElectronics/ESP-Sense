var loginFormHtml = `<div id="auth_con" class="container">
    <form id="login_form" class="login-box">
        <h4>Authentication</h4><input id="user_input" class="form-control" type="text" placeholder="Username" /><input id="pass_input" class="form-control" type="password" placeholder="Password" /><button id="login_btn" class="btn btn-primary" type="submit">Login</button><span id="login_msg" style="display: none;">Incorrect Password!</span>
    </form>
</div>`;

var espAuthenticated = false;

//function validateAuthForm(){
//    console.log('validate');
//    
//    if($('#user_input').val().length == 0 || $('#pass_input').val().length == 0){
//        //disableObj($('#login_btn'));
//        return false;
//    }
//    //enableObj($('#login_btn'));
//    return true;
//}

function authSuccess() {
    espAuthenticated = true;
    Show($('.main-container'));
    $('#auth_con').remove();
    console.log('Authorization Successful');
    $('html').trigger('auth');
    $('html').attr('auth', '');
    $('#logout_but').click(logout);
}

function authFailed(request, status, error) {
    console.log('Authorization Failed');

    $('.main-container').remove();

    if ($('#auth_con').length == 0) {
        $('#nav_head').after(loginFormHtml);
    } 
    attachFormEvent();
    Show($('#auth_con'));
}

function dec2bin(dec){
    return (dec >>> 0).toString(2);
}

function strToByteArray(str){
    var arr = [str.length];
    
    for(var i = 0; i < str.length; i++){
        arr[i] = str[i].charCodeAt(0);
    }
    
    return arr;
}

function printBits(str) {
    console.log('print bits ' + str);
    var bitties = '';
    for (var i = 0; i < str.length; i++) {
        console.log(str[i] + ' : '+ str[i].charCodeAt(0) + ' : 0x'+ str[i].charCodeAt(0).toString(16) + ' : ' + dec2bin(str[i].charCodeAt(0) >>> 0).toString(2));
        //for (var b = 0; b < 8; b++) {
        //    var bit = 0;
        //    
        //    bit = ((str[i].charCodeAt(0) >> b) & 1) + 48;
        //    bitties += String.fromCharCode(bit);
        //}
        
    }
}

function attachFormEvent(){
    $('#login_form').submit(function(e) {
        e.preventDefault();
        //if(!validateAuthForm()) return;
        console.log("Login Form Submit");

        var jdata = {};
        jdata["username"] = strToByteArray(encryptXOR($('#user_input').val(), 0xAA));
        jdata["password"] = strToByteArray(encryptXOR($('#pass_input').val(), 0xAA));
        
        //printBits(jdata["password"]);
        //printBits(encryptXOR(jdata["password"], 0xAA));
        console.log(jdata["username"]);
        console.log(jdata["password"]);
        //console.log(encryptXOR(jdata["username"], 0xAA));
        //console.log(encryptXOR(jdata["password"], 0xAA));

        var jStr = JSON.stringify(jdata);
        
        printBits(jStr);

        console.log(jStr);

        $.ajax({
            url: "/auth",
            type: 'POST',
            data: jStr,
            username: jdata["username"],
            password: jdata["password"],
            success: function() {
                console.log("Authentication Successful");
                refreshPage();
            },
            error: function(request, status, error) {
                console.log("Invalid username or password");
                $('#login_msg').text("Invalid Username or Password");
                Show($('#login_msg'));
            }
        });
    });
}

function authenticate() {
    console.log("Authenticating...");

    $.ajax({
        type: 'GET',
        url: "/auth",
        data: null,
        success: authSuccess,
        error: authFailed
    });
}

function logout() {
    console.log("Logging out...");

    $.ajax({
        type: 'GET',
        url: "/logout",
        data: null,
        success: refreshPage,
        error: function(request, status, error){
            console.log('Logout Failed');
        }
    });

}

function encryptXOR(msg, key) {
    var enc = '';
    var k = isNaN(key) ? key.charCodeAt(0) : key;

    for (var i = 0; i < msg.length; i++) {
        var c = msg.charCodeAt(i);
        var x = (c ^ k) ;
        enc += String.fromCharCode(x);
    }

    return enc;
}



$(document).ready(function() {
    $('.main-container').hide();
    $('#auth_con').hide();
    //$('#user_input').change(validateAuthForm);
    //$('#pass_input').change(validateAuthForm);
    //disableObj($('#login_btn'));
    authenticate();
});