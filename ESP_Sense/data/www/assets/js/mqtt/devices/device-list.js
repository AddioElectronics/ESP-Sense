
var canSelectDevice = true;
var selectedDeviceElem = null;
//var deleteOld = false;


var testhtml = `<span name>Co2 Sensor</span><span device>SCD4x<br /></span><span status="ok"><br /></span>`
var navToDevButHtml = `<button class="btn btn-primary" type="button" action="select-device" disabled>Navigate to</button>`;

var deviceListElem = $('main [mqtt=devicelist]');

var navToDeviceButton = $('main button[action=select-device]');
var navToDevVisDetector = $('div.vis-detect[button=select-device]');

var clickedDev = false;
var navingToDev = false;

function navToDevButClick(){
    if(MqttDevice.activeDevice == null)
        return;
    MqttDevice.NavigateToActive();
}

function initNavToDevBut(){
    console.log('initNavToDevBut');
    if(navToDeviceButton.length == 0) return;
    
    if(MqttDevice.activeDevice == null){
        disableObj(navToDeviceButton);
        navToDeviceButton.text('Select A Device');
    }else{
        enableObj(navToDeviceButton);
        navToDeviceButton.text('Navigate to ' + MqttDevice.activeDevice.name);
        $('.grid.device-container[mqttdevice] [name='+MqttDevice.activeDevice.name+']').parent().attr('selected', '');
        loadMqttDeviceSummary();
    }
    
    navToDeviceButton.click(MqttDevice.NavigateToActive);
    
    return;
    
    if(navToDeviceButton.length == 0 && deviceListElem.length > 0){
        $('main [mqtt=devicelist]').prepend(navToDevButHtml);
        navToDeviceButton = $('main button[action=select-device]');   
    }

    disableObj(navToDeviceButton);

    if(navToDevVisDetector.length == 0 && deviceListElem.length > 0){
        $('main [mqtt=devicelist]').prepend(createElement('div', null, 'vis-detect', {key:'button', value:'select-device'}));
        navToDevVisDetector = $('div.vis-detect[button=select-device]');  
    }
    
    navToDeviceButton.click(MqttDevice.NavigateToActive);
    //moveNavToDevButton(null);
}



function moveNavToDevButton(e){
    var offset = {top:-$('nav').height(), bottom:-$('footer').height()};
    if(navToDevVisDetector.isInViewport(offset)){
        if(!navToDeviceButton.hasClass('position-absolute'))
            navToDeviceButton.addClass('position-absolute');
    }else{
        
        navToDeviceButton.removeClass('position-absolute');
        navToDeviceButton.css('--device-status-width', deviceListElem.offset().left+20+'px');
    }
    
    if(!deviceListElem.isInViewport())
        navToDeviceButton.hide();
    else
        navToDeviceButton.show();    
}

//EventWindowScroll.add(moveNavToDevButton);

function createDeviceList(devices){
    console.log('createDeviceList()')
    console.log(devices);
    
    var dlistRoot = $('[mqtt=devicelist]');
    
    //if(deleteOld)
    //dlistRoot.find('.grid.list').remove();

    var dlistcon = dlistRoot.find('.grid.list');// createElement('div', null, ['grid', 'list']);
    $(dlistRoot.find('.heading')[0]).after(dlistcon);
    
    var senList = createSublist('Sensors', 'sensors', devices.sensors);
    var binsenList = createSublist('Binary Sensors', 'binarySensors', devices.binarySensors);
    var ledList = createSublist('Lights', 'lights', devices.lights);
    var butList = createSublist('Buttons', 'buttons', devices.buttons);
    var swList = createSublist('Switches', 'switches', devices.switches);
    
    dlistcon.append(senList);
    dlistcon.append(binsenList);
    dlistcon.append(ledList);
    dlistcon.append(butList);
    dlistcon.append(swList);
    //MqttDevice.eventStatusChanged.Invoke();
    MqttDevice.SetAllDeviceLeds();
    MqttDevice.SetAllLedHeadGroups();
    //console.warn('working');
    dlistcon.keypress(function(e){        
        if($(document.activeElement).attr('disabled')) return;
        selectMqttDevice(e);
    });
    dlistcon.click(async function(e){
        if(clickedDev) return;
        clickedDev = true;
        await sleep(300);
        if(e.originalEvent.detail > 1 || navingToDev){
            //Because of cache need to set to false.
            //Need to edit cache control on server
            navingToDev = false;
            clickedDev = false;
            return;
        }
        
        if(!navingToDev){
            selectMqttDevice(e);
        }
        clickedDev = false;

    });
    dlistcon.dblclick(async function(e){
        navingToDev = true;
        //disableDeviceList();
        selectMqttDevice(e);
        MqttDevice.NavigateToActive();
    });
}

function createSublist(name, dtype, devices){
    console.log('createSublist() ' + name);
    var devGroup = createElement('div', null, ['device-group'],{key:'device-type', value:dtype} );
    let headCon = createElement('div', null, ['grid', 'head-container']);
    
    var heading = createElement('h3', null, null, null, name);
    var ledGroup = MqttDevice.CreateLedController(true, true);
    var listCon = createElement('div', null, 'sub-list');
    
    ledGroup.attr('device-type', dtype);
    
    headCon.append(heading);
    headCon.append(ledGroup);
    devGroup.append(headCon);
    devGroup.append(listCon);
    
    if(devices == null || devices.length == 0) return;
    
    listCon.prepend(createHeadingForDeviceList());
    
    for(var i = 0; i < devices.length; i++){
        var devElem = createDeviceForList(devices[i]);
        listCon.append(devElem);
    }
    console.log(devGroup);
    return devGroup;
}

function createHeadingForDeviceList(dev){
    console.log('createHeadingForDeviceList()');
    var headElem = createElement('div',
                                null, 
                                ['grid', 'label-container']);
        
    var nameElem = createElement('span', null, null, null, 'Name')
    var dtypeElem = createElement('span', null, null, null, 'Device')
    var stateElem = createElement('span', null, null, null, 'Status')
    
    headElem.append(nameElem);
    headElem.append(dtypeElem);
    headElem.append(stateElem);
    
    return headElem;
}

function createDeviceForList(dev){
    console.log('createDeviceForList()');
    console.log(dev);
    let devElem = createElement('div',
                                null, 
                                ['grid', 'device-container'],
                                [{key:'tabindex', value:0},{key:'mqttdevice'}, {key:'url', value:dev.url}] );
    
    let nameElem = createElement('span', null, null, {key:'name', value:dev.name}, dev.name);
    let dtypeElem = createElement('span', null, null, [{key:'device', value:dev.device}, {key:'type', value:dev.type}], dev.device);

    let stateElem = MqttDevice.CreateLedController(true, true, true);        
    dev.ledGroup = stateElem;
    stateElem.attr('dev-index', dev.index);
    stateElem.attr('dev-type', dev.type);
    dev.listElem = devElem;
    
    
    devElem.attr('dev-index', dev.index);
    devElem.attr('dev-type', dev.type);
    
    devElem.append(nameElem);       
    devElem.append(dtypeElem);
    devElem.append(stateElem);
        
    return devElem;
}

function selectMqttDevice(e){
    var elem = e.type == 'click' ? $(e.target) : $(document.activeElement);
    
    if(elem.is('led'))return null;
    if(elem.attr('disabled') != undefined) return null;
    
    if(elem.attr('mqttdevice') == null){
        if(elem.parent().attr('mqttdevice') == null) return null;        
        elem = elem.parent();
    }
    
    disableDeviceList();
    
    console.log('selectMqttDevice()');
    console.log(elem);   
    
    $('[mqttdevice]').removeAttr('selected');
    
    elem.attr('selected', '');
    selectedDeviceElem = elem;
    let name = elem.find('[name]').attr('name');
    let type = elem.find('[type]').attr('type');
    let device = MqttDevice.Find(name, type, true);
    
    if(device == null){
        selectedDeviceElem = null;
        console.log('Could not find device! name : ' + name + ' | type : ' + type);
        navToDeviceButton.text('Error');
        ensableDeviceList();
        return null;
    }
    
        
    navToDeviceButton.text('Navigate to ' + elem.find('[name]').attr('name'));

    loadMqttDeviceSummary();
    
    enableDeviceList();
    
    return elem;
}

function loadMqttDeviceSummary(){
    if(devMode){
        loadDummyMqttStatus();
    }else{
        disableDeviceList();
        MqttDevice.activeDevice.GetStatus(function(data){
            enableDeviceList();
            setMqttDeviceStatus(data, MqttDevice.activeDevice);
        }, function(xhr, status, error){
            enableDeviceList();
            let device = MqttDevice.activeDevice;
            $('[status-content=mqttdevice]').text('Could not load ' + device.name + '(' + device.device + ') status | ' + xhr.responseText);
        });
    }
}

function disableDeviceList(){
    disableObj($('section[mqtt=devicelist] *'), true);
    disableObj(navToDeviceButton);
}

function enableDeviceList(){
    enableObj($('section[mqtt=devicelist] *'), true);
    enableObj(navToDeviceButton);
}


onEvent('auth', function(){
    //initNavToDevBut();
});