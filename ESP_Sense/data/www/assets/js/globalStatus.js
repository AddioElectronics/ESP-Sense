var globalStatusJOBJ;

function globalStatusInit() {
    console.log("globalStatusInit()");
    navWebUpdate = $('#nav_webupdate');
    
    if(devMode){
        loadDummyGlobalStatus();
    }else{
        requestJsonData("/status", 
                        receivedGlobalStatus, 
                        retryGetGlobalStatus, 
                        null, 
                        5000); 
    }       
}

function receivedGlobalStatus(data){
    console.log("Received Global Status :");
    console.log(data);
    globalStatusJOBJ = data;
    
    //configMode = data['status']['status']['device']['configMode'];
    configMode = data['status']['status']['wifi']['configMode'];    
    
    populateGlobalStatus(data);
    invokeEvent('globalStatus');
    
    if(configMode)
        invokeEvent('configMode');
}

function populateGlobalStatus(data){
    
    let statusObj = data['status']['status'];
    let retainedObj = data['status']['retainedStatus'];
    if ($('[status-content=global]').length > 0){
        let gsobj = $('[status-content=global]'); 
        //objectToTables(gsobj, statusObj, true);
        objectToTables(gsobj, data, false);
    }
       
    //if ($('[status-content=retain]').length > 0){
        //let rsobj = $('[status-content=retain]'); 
        //objectToTables(rsobj, retainedObj, true);
    //}  
}

function retryGetGlobalStatus(request, status, error) {
    console.log('Could not get Global Status.');
    $('#global_status').text('Could not get global status');

    //setTimeout(function(){
    //    retryGetDeviceStatus();
    //}, 2000);
}


function loadDummyGlobalStatus(){
    loadData('/assets/js/dummyStatus.json', function(data){
        receivedGlobalStatus(data);
        //let statusObj = data['status'];
        //let retainedObj = data['retainedStatus'];
        //let gsobj = $('[status-content=global]');
        
        //objectToTables(gsobj, data, false, [{key:'action', value:'hide-content'}, {key:'actionid', value:'status'}])
        //objectToTables(gsobj, statusObj)
        //objectToTables(gsobj, retainedObj)

    });
}

EventReady.add(function(){
    if ($('#global_status') == null) return;
    onEvent('auth', globalStatusInit);
});