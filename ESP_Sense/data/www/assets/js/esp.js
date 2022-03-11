var devMode = true;

var devModeLabel = '<span class="dev-mode">Developer Mode</span>';
var rstHtml = `<button id="esp_reset_button" class="btn btn-primary" type="button" disabled>Reset ESP</button>`;

var rstBut;

function espControllerInit() {
    console.log('espControllerInit');
    if($('#esp_reset_button').length == 0){
        $('body').append(rstHtml);
    }
    Show(rstBut);
    onEvent('auth', function(){
       enableObj(rstBut); 
    });
    rstBut.click(espReset);
}

function espReset() {
    if (updateInProgess || !espAuthenticated) return;
    console.log('Sending Reset Request...');
    $.ajax({
        type: 'POST',
        url: "/reset",
        data: null,
        success: espIsResetting,
        error: function(a, b, c) {
            console.log('Reset Failed');
        }
    });
}

//error request, status, error
function espAlive(success, error){
    $.ajax({
            type: 'GET',
            url: "/alive",
            data: null,
            success: success,
            error:error
    });
}

function refreshOnResponse(){
    setTimeout(function(){
            espAlive(refreshPage, function(request, status, error){refreshOnResponse();});
    }, 5000);
}

function espIsResetting(){
    console.log('Resetting ESP');
    enableBlockingMsg('Resetting ESP...');
    refreshOnResponse();
}

function getEspVersion() {
    console.log("deviceStatusInit()");
    navWebUpdate = $('#nav_webupdate');
    requestJsonData("/version", receivedVersion, function(){
        console.log('GET version failed.')
    });    
}

function receivedVersion(data){
    console.log("Received Version :");
    console.log(data);
    espVersion = data['version'];
                        
    if ($('#global_status').length > 0) {
        $('#global_status').append(syntaxHighlight(JSON.stringify(deviceStatusJOBJ, null, 4)));
    }
    
    var verobj = $('.version');
    verobj.text(verobj.text().replace('x.x.x', espVersion.toString()));
    
    invokeEvent('version');
}

EventReady.add(function(){
    espAlive(function(){devMode = false;});
    rstBut = $('#esp_reset_button');
    console.log(rstBut);
    rstBut.hide();
    onEvent('resetting', espIsResetting);
    espControllerInit();
    getEspVersion();
    
    if(devMode){
      $('body').append(devModeLabel);
}
});