class MqttDeviceInfo {
    constructor(name, device, type, state) {
        this.name = name;
        this.device = device;
        this.type = type;
        this.state = state;
        this.url = '/mqtt/devices/' + type + '/' + name;
        this.path = '/mqtt/devices/' + type + '/' + device;
        console.log('MqttDeviceInfo() name : ' + this.name + ' | device : ' + this.device + ' | type : ' + type + ' | state : ' + this.state + ' | url : ' + this.url)
    }
}

var mqttDevicesInfoJOBJ;

function devicesInfoInit() {
    console.log('devicesInfoInit()');

    requestJsonData('/mqtt/devices/info',
        function(data) {
            console.log('Received MQTT Device Info :');
            console.log(data);
            mqttDevicesInfoJOBJ = data;
            processMqttInfo();
        },
        retryGetDevicesInfo);
        
    
    $('[action=select-device]').click(navigateToSelectedDevice);
}


function processDevInfo(type) {
    console.log('processDevInfo()');
    if (mqttDevicesInfoJOBJ['mqttDeviceInfo'][type]) {
        var devs = mqttDevicesInfoJOBJ['mqttDeviceInfo'][type];
        for (var i = 0; i < devs.length; i++) {
            var d = devs[i];
            mqttDevices[type].push(new MqttDeviceInfo(d['name'], d['device'], type, d['state']));
        }
    }
}

function processMqttInfo() {
    console.log('processMqttInfo()');
    setMqttDeviceSummary();
    processDevInfo('sensors');
    processDevInfo('binarySensors');
    processDevInfo('switches');
    processDevInfo('buttons');
    processDevInfo('lights');

    if($('[mqtt=devicelist]').length > 0){
        createDeviceList(mqttDevices);
    }
}

function setMqttDeviceSummary() {
    console.log('setMqttDeviceSummary()');
    if ($('#info_devices').length > 0) {
        $('#info_devices').append(syntaxHighlight(JSON.stringify(mqttDevicesInfoJOBJ, null, 4)));
    }
}

function retryGetDevicesInfo(request, status, error) {
    console.log('retryGetDevicesInfo()');
    console.log('Could not get MQTT Devices Info.');
    $('#info_devices').text('Could not get MQTT Devices Info.');

}

function loadDummyMqttDeviceInfo(){
    console.log('loadDummyMqttDeviceInfo()');
    loadData('/assets/js/mqtt/dummy-device-list-test.json', function(data){
        mqttDevicesInfoJOBJ = data;
        processMqttInfo();
    });
}


EventReady.add(function(){
    if ($('[mqtt]') == null) return;
    onEvent('auth', devicesInfoInit);
    if(devMode)
        loadDummyMqttDeviceInfo();
});