function setDeviceInfoElement(parent, type, text, pre, ignoreAttr, ignoreText){
    var elems = $(parent).find('['+type+']');//$('span['+type+'], h1['+type+'], h2['+type+'], h3['+type+'], h4['+type+'], h5['+type+'], h6['+type+']');
    
    if(!ignoreText && elems.is('span, h1, h2, h3, h4, h5, h6'))
    elems.text((pre != null ? pre : '') + text);
    
    if(!ignoreAttr)
    elems.attr(type, text);
}

function setMqttDeviceStatus(data, device){

    if($('section.device-status').length == 0) return;
    
    //Dummy data
    if(device == undefined){
        device = MqttDevice.activeDevice;
    }
    
    if(data == null){
        data = device.status;
    }

    let key = Object.keys(data)[0];
    let rootObj = data[key];
    //If dev-index/type empty, there is only one device that can be displayed
    let dsobj = $('section.device-status[dev-index=""][dev-type=""] [status-content=mqttdevice], section.device-status[dev-index='+device.index+'][dev-type='+device.type+'] [status-content=mqttdevice]');
    objectToTables(dsobj, data, false);
    disableBlockingMsg();
    
    if($('.device-config').length == 0) return;
    
    MqttDevice.activeDevice.GetConfig(setConfigForm, function(xhr, status, error){
        $('[config-content=mqttdevice]').text("Get Status Failed...");
    });
    
    return;
    populateStatusTable(dsobj.find('[status-content=device]').find('tbody'), rootObj['deviceStatus']);
    populateStatusTable(dsobj.find('[status-content=deviceType]').find('tbody'), rootObj['sensorStatus']);
    populateStatusTable(dsobj.find('[status-content=unique]').find('tbody'), rootObj['uniqueStatus']);
    populateStatusTable(dsobj.find('[status-content=website]').find('tbody'), rootObj['websiteStatus']);
}

function submitMqttDeviceConfigForm(){    
    if(MqttDevice.activeDevice == null) return;
    
    var data = null;
    
    setBlockingMsg('Saving ' + MqttDevice.activeDevice.name + ' Config...', 5000);
    MqttDevice.activeDevice.SetConfig(data,submitConfigSuccess, submitConfigFailed);
}


function submitConfigSuccess(){
    setBlockingMsg('Save Complete!', 3000);
}

function submitConfigFailed(xhr, status, error){
    setBlockingMsg('Save Failed!', 3000);
}

function setConfigForm(data){
    let key = Object.keys(data)[0];
    let rootObj = data[key];
    let formObj = $('form[form-action=device-config]');
    
    formObj.load(MqttDevice.activeDevice.url);
    
    formObj.submit(submitMqttDeviceConfigForm);
}

function loadDummyMqttStatus(){
    loadData('/assets/js/mqtt/dummy-status-test.json',setMqttDeviceStatus);
}

function loadDummyMqttConfig(){
    //loadData('/assets/js/mqtt/dummy-status-test.json',);
}

//Set led function, (state, events)


function devicePageInit(){
    if($('main[contents=mqttdevice]').length == 0) return;
    getMqttDeviceData();
}



function getMqttDeviceData(){   
    enableBlockingMsg('Loading Device Info...');
    
    //Will get config on success
    var device = MqttDevice.GetActiveDevLocal();

    device.ledGroup = $('ledgroup');
    device.SetLeds();
    device.ledGroup.attr('dev-index', device.index);
    device.ledGroup.attr('dev-type', device.type);
    
    MqttDevice.SetActiveDevice(device);
    
    let deviceHeading = $('[contents=mqttdevice] .device-heading');
    
    setDeviceInfoElement(deviceHeading, 'name', capitalFirst(device.name));
    setDeviceInfoElement(deviceHeading, 'device', device.device.toUpperCase(), 'Device : ', true);
    setDeviceInfoElement(deviceHeading, 'state', device.state, null, true);
    //setDeviceInfoElement(deviceHeading, 'state', device.state, null, false, true);
    
    //$('[name]').text(capitalFirst(device.name));
    //$('[state]').text('State : Online');
    //$('[state]').attr('state', device.state);
    //$('[device]').text('Device : ' + device.device.toUpperCase());
    
    if(devMode){
        loadDummyMqttStatus();
        loadDummyMqttConfig();
        return;
    }
 
    
    
    if(device == null){
        //Go back to device list
        window.location.href = '/mqtt/devices.html';
        return;
    }
    
    device.GetStatus(setMqttDeviceStatus, function(xhr, status, error){
        let err = xhr.responseText;
        enableBlockingMsg('Could not load ' + device.name + '(' + device.device + ') status\r\n' + err, true, 'red');
        $('[status-content=mqttdevice]').text("Get Status Failed...");
    });
    
    //Called in setMqttDeviceStatus
    //device.GetConfig(setConfigForm, function(xhr, status, error){
    //    var err = eval("(" + xhr.responseText + ")");
    //    enableBlockingMsg('Could not load ' + device.name + '(' + device.device + ') config\n' + err);
    //});
}

MqttDevice.eventDeviceInfoCreated.add(function(){    
    devicePageInit();     
});
//EventReady.add(function(){
    
    //onEvent('auth', devicePageInit);
     
//});