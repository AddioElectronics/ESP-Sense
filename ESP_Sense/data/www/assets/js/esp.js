var devMode = true;
var keepTest = devMode && false;
var alwaysLoadPage = false;

var devModeLabel = '<span class="dev-mode">Developer Mode</span>';
var rstHtml = `<button id="esp_reset_button" class="btn btn-primary" type="button" disabled>Reset ESP</button>`;

var rstBut;

createEvent('version');
createEvent('auth');
createEvent('reset');
createEvent('resetting');
createEvent('status');
createEvent('globalStatus');
createEvent('configMode');

if(!keepTest)
$('.test').remove();

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
    enableBlockingMsg('Resetting ESP...', true);
    $('*').click(function(){return false;})  
    refreshOnResponse();
}

function getEspVersion() {
    console.log("getEspVersion()");
    navWebUpdate = $('#nav_webupdate');
    requestJsonData("/version", 
                          receivedVersion, 
                          function(){
        console.log('GET version failed.');
        invokeEvent('version');
    },
                         null,
                         5000,      //Tout
                         false);    //Async 
}

function receivedVersion(data){
    devMode = false;
    console.log("Received Version :");
    console.log(data);
    espVersion = data['version'];
                        
    if ($('#global_status').length > 0) {
        $('#global_status').append(syntaxHighlight(JSON.stringify(deviceStatusJOBJ, null, 4)));
    }
    
    var verobj = $('.version');
    verobj.text(verobj.text().replace('x.x.x', espVersion.toString().replace(',', '.')));
    
    invokeEvent('version');
}

function addAlwaysLoadPage() {
    window.addEventListener("pageshow", function(event) {
        var historyTraversal = event.persisted ||
            (typeof window.performance != "undefined" &&
                window.performance.navigation.type === 2);
        if (historyTraversal) {
            // Handle page restore.
            window.location.reload();
        }
    });
}

if(alwaysLoadPage)
    addAlwaysLoadPage();


EventReady.add(async function(){
    await espAlive(function(){devMode = false;});
    await getEspVersion();
    rstBut = $('#esp_reset_button');
    console.log(rstBut);
    rstBut.hide();
    onEvent('resetting', espIsResetting);
    espControllerInit();
    
    
    if(devMode){
      $('body').append(devModeLabel);
}
});




