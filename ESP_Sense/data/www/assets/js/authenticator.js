var loginFormHtml = `<div id="auth_con" class="container">
    <form id="login_form" class="login-box">
        <h4>Authentication</h4><input id="user_input" class="form-control" type="text" placeholder="Username" /><input id="pass_input" class="form-control" type="password" placeholder="Password" /><button id="login_btn" class="btn btn-primary" type="submit">Login</button><span id="login_msg" style="display: none;">Incorrect Password!</span>
    </form>
</div>`;

var espAuthenticated = false;

class Authenticator{
    static authenticated = false;
    
    static formHtml = `<div id="auth_con" class="container">
    <form id="login_form" class="login-box">
        <h4>Authentication</h4><input id="user_input" class="form-control" type="text" placeholder="Username" /><input id="pass_input" class="form-control" type="password" placeholder="Password" /><button id="login_btn" class="btn btn-primary" type="submit">Login</button><span id="login_msg" style="display: none;">Incorrect Password!</span>
    </form>
</div>`;
    
    static Start(){
        console.log("Authenticating...");
        $.ajax({
            type: 'GET',
            url: "/auth",
            data: null,
            success: Authenticator.Success,
            error: devMode ? Authenticator.Success : Authenticator.Failed
        });
    }
    
    static Logout(){
        console.log("Logging out...");

        $.ajax({
            type: 'POST',
            url: "/logout",
            data: null,
            success: refreshPage,
            error: function(request, status, error){
                console.log('Logout Failed');
            }
        });
    }
    
    static Is(){
        return Authenticator.authenticated;
    }
    
    static Success(){
        Authenticator.authenticated = true;
        Show($('main'));
        $('#auth_con').remove();
        console.log('Authorization Successful');
        invokeEvent('auth');
        $('html').attr('auth', '');
        $('#logout_but').click(Authenticator.Logout);
    }
    
    static Failed(){
        console.log('Authorization Failed');

        $('main').remove();

        if ($('#auth_con').length == 0) {
            $('#nav_head').after(Authenticator.formHtml);
        } 
        Authenticator.AttachFormEvent();
        Show($('#auth_con'));
    }
    
    //static ValidateForm(){
        //console.log('validate');
        //    
        //if($('#user_input').val().length == 0 || $('#pass_input').val().length == 0){
        //    //disableObj($('#login_btn'));
        //    return false;
        //}
        ////enableObj($('#login_btn'));
        //return true;
    //}
    
    static AttachFormEvent(){
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
                username: jdata["username"],    //Not used anymore
                password: jdata["password"],    //Not used anymore
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
}

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



onEvent('version', function(){
    $('.main-container').hide();
    $('#auth_con').hide();
    //$('#user_input').change(validateAuthForm);
    //$('#pass_input').change(validateAuthForm);
    //disableObj($('#login_btn'));
    Authenticator.Start();
});