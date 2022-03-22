/*Global vars for browser*/
var devMode = false;
var keepTest = devMode && false;
var alwaysLoadPage = false;
var websiteVersion = [0,1,8];
var githubReleasesUrl = 'https://github.com/AddioElectronics/ESP-Sense/releases';

/*Global Vars Retrieved from ESP*/
var espVersion = [0,0,0];   //Firmware version
var configMode = false;     //Is the ESP currently in configMode
var globalStatusJOBJ;       //Global status of the ESP

var mqttDevices = {sensors:[], binarySensors:[], lights:[], buttons:[], switches:[]};




var devModeLabel = '<span class="dev-mode">Developer Mode</span>';
var rstHtml = `<button id="esp_reset_btn" class="btn" type="button" disabled>Reset ESP</button>`;

var rstBut;

createEvent('version');
createEvent('auth');
createEvent('reset');
createEvent('resetting');
createEvent('status');
createEvent('globalStatus');
createEvent('configMode');

//Remove all elements with the "test" class.
if(!keepTest)
$('.test').remove();

//Initialize the ESP control panel.
//*Unfinished
//Create button to reset device.
function espControllerInit() {
    console.log('espControllerInit');
    if($('#esp_reset_btn').length == 0){
        $('body').append(rstHtml);
        rstBut = $('#esp_reset_btn');
    }
    onEvent('auth', function(){
       enableObj(rstBut); 
    });
    rstBut.on('click', espReset);
}

//Send a reset request to the ESP.
function espReset() {
    if (updateInProgess || !Authenticator.authenticated) return;
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

//Check to see if the ESP is still on and server is running.
//Used to determine when the ESP is online after a reset.
//param error : (request, status, error)
function espAlive(success, error){
    $.ajax({
            type: 'GET',
            url: "/alive",
            data: null,
            success: success,
            error:error
    });
}

//Refresh when espAlive is a success.
function refreshOnResponse(){
    setTimeout(function(){
            espAlive(refreshPage, function(request, status, error){refreshOnResponse();});
    }, 5000);
}

//Called when ESP has received the reset request has
//sent an acknowledgement.
//Displays a message and refreshes page when ESP is online.
function espIsResetting(){
    console.log('Resetting ESP');
    enableBlockingMsg('Resetting ESP...', true);
    $('*').click(function(){return false;})  
    refreshOnResponse();
}

//Request the firmware version from the ESP.
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

//Received the firmware version from the ESP.
function receivedVersion(data){
    devMode = false;
    console.log("Received Version :");
    console.log(data);
    espVersion = data['version'];
    configMode = data['configMode'];
    
    if ($('#global_status').length > 0) {
        $('#global_status').append(syntaxHighlight(JSON.stringify(deviceStatusJOBJ, null, 4)));
    }
    
    var verobj = $('.version');
    verobj.text(verobj.text().replace('x.x.x', String.format("{0}.{1}.{2}", espVersion[0], espVersion[1], espVersion[2])));
    
    invokeEvent('version');
    
    var latest = getLatestVersion();
    
    if(compareVersions(espVersion, websiteVersion) == -1){
        addNotification('fa fa-code', 'Update Available', function(){
            //window.location.href = '/update.html';                        //Go to update page (one-click update not implemented)
            openInNewTab('https://github.com/AddioElectronics/ESP-Sense');  //Go to github page
        });
        addNotification('fa fa-code', 'Firmware and Website Version do not match', function(){openInNewTab(githubReleasesUrl);});
    }
    
    if(compareVersions(espVersion, websiteVersion) != 0){
        addNotification('fa fa-code', 'Firmware and Website Version do not match', function(){openInNewTab(githubReleasesUrl);});
    }
}

function compareVersions(a, b){
    for(var i = 0 ; i < 3; i++){
        if(a[i] < b[i]) return -1;
        if(a[i] > b[i]) return 1;
    }
    return 0;
}

function getLatestVersion(){
    return espVersion;
}

//Forces page to reload and ignore cache
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
    rstBut = $('#esp_reset_btn');
    rstBut.hide();
    console.log(rstBut);
    espControllerInit();
    await espAlive(function(){devMode = false;});
    await getEspVersion();
    onEvent('resetting', espIsResetting);
    
    if(devMode){
      $('body').append(devModeLabel);
        
    
}
});




