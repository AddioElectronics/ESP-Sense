var deviceStatusJOBJ;

function deviceStatusInit() {
    console.log("deviceStatusInit()");
    navWebUpdate = $('#nav_webupdate');
    requestJsonData("/status", receivedDeviceStatus, retryGetDeviceStatus);    
}

function receivedDeviceStatus(data){
    console.log("Received Device Status :");
    console.log(data);
    deviceStatusJOBJ = data;
                        
    if ($('#global_status').length > 0) {
        $('#global_status').append(syntaxHighlight(JSON.stringify(deviceStatusJOBJ, null, 4)));
    }
    
    invokeEvent('deviceStatus');
}

function retryGetDeviceStatus(request, status, error) {
    console.log('Could not get Device Status.');
    $('#global_status').text('Could not get global status');

    //setTimeout(function(){
    //    retryGetDeviceStatus();
    //}, 2000);
}

function loadDummyStatus(){
    loadData('/assets/js/dummyStatus.json', function(data){
        var rootObj = data['status'];
        var dso = $('.device-status');
        let tableElem = createStatusTable('global');
        populateStatusTable(dso.find('[status-content=global]').find('tbody'), rootObj);

    });
}

EventReady.add(function(){
    if ($('#global_status') == null) return;
    onEvent('auth', deviceStatusInit);
});