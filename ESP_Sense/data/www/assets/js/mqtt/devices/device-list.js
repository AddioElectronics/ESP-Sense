
var testhtml = `<span name>Co2 Sensor</span><span device>SCD4x<br /></span><span status="ok"><br /></span>`
var navToDevButHtml = `<button class="btn btn-primary" type="button" action="select-device" disabled>Navigate to</button>`;

var selectedDeviceElem = null;
var deleteOld = false;

var deviceListElem = $('main [mqtt=devicelist]');

var navToDeviceButton = $('main button[action=select-device]');
var navToDevVisDetector = $('div.vis-detect[button=select-device]');

function initNavToDevBut(){
    if($('main [mqtt=devicelist]').length == 0) return;
    
    if(navToDeviceButton.length == 0 && deviceListElem.length > 0){
        $('main [mqtt=devicelist]').prepend(navToDevButHtml);
        navToDeviceButton = $('main button[action=select-device]');   
    }

    disableObj(navToDeviceButton);

    if(navToDevVisDetector.length == 0 && deviceListElem.length > 0){
        $('main [mqtt=devicelist]').prepend(createElement('div', null, 'vis-detect', {key:'button', value:'select-device'}));
        navToDevVisDetector = $('div.vis-detect[button=select-device]');  
    }
    
    navToDeviceButton.click(navigateToSelectedDevice);
    //moveNavToDevButton(null);
}



function moveNavToDevButton(e){
    if(navToDevVisDetector.isInViewport()){
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

EventWindowScroll.add(moveNavToDevButton);

function createDeviceList(devices){
    console.log('createDeviceList()')
    console.log(devices);
    
    var dlistRoot = $('[mqtt=devicelist]');
    
    if(deleteOld)
    dlistRoot.find('.grid.list').remove();

    var dlistcon = createElement('div', null, ['grid', 'list']);
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
    
    
    dlistcon.keypress(selectMqttDevice);
    dlistcon.click(selectMqttDevice);
}

function createSublist(name, dtype, devices){
    console.log('createSublist() ' + name);
    var devGroup = createElement('div', null, ['device-group'],{key:'device-type', value:'sensors'} );
    var heading = createElement('h3', null, null, null, name);
    var listCon = createElement('div', null, 'sub-list');
    
    devGroup.append(heading);
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
    var devElem = createElement('div',
                                null, 
                                ['grid', 'device-container'],
                                [{key:'tabindex', value:0},{key:'mqttdevice'}, {key:'url', value:dev.url}] );
    
    var nameElem = createElement('span', null, null, {key:'name', value:dev.name}, dev.name)
    var dtypeElem = createElement('span', null, null, [{key:'device', value:dev.device}, {key:'type', value:dev.type}], dev.device)
    var stateElem = createElement('span', null, null, {key:'state', value:dev.state})
    
    devElem.append(nameElem);       
    devElem.append(dtypeElem);
    devElem.append(stateElem);
    
    return devElem;
}

function selectMqttDevice(e){
    var elem = e.type == 'click' ? $(e.target) : $(document.activeElement);
    
    if(elem.attr('mqttdevice') == null){
        if(elem.parent().attr('mqttdevice') == null) return null;
        
        elem = elem.parent();
    }
    
    console.log('selectMqttDevice()');
    console.log(elem);   
    
    disableObj(navToDeviceButton);
    
    if(selectedDeviceElem != null)
        selectedDeviceElem.removeAttr('selected');
    
    elem.attr('selected', '');
    selectedDeviceElem = elem;
    //enableObj(navToDeviceButton);
    
    //console.log(navToDeviceButton);
    //disableObj(navToDeviceButton);
    enableObj(navToDeviceButton);
    //navToDeviceButton.hide();
    
    navToDeviceButton.text('Navigate to ' + selectedDeviceElem.find('[name]').attr('name'));
    
    return elem;
}

function navigateToDevicePage(device){
    console.log('navigateToDevicePage()');
    localStoreObj('device', device);
    window.location.href = device.info.path + '.html';
}

function navigateToSelectedDevice(){
    console.log('navigateToSelectedDevice()');
    if(selectedDeviceElem == null) return;    
    console.log(selectedDeviceElem);
    var name = selectedDeviceElem.find('[name]').attr('name');
    var type = selectedDeviceElem.find('[type]').attr('type');
    var device = findMqttDevice(name, type);
    
    if(device == null){
        console.log('Could not find device! name : ' + name + ' | type : ' + type);
        return;
    }
    
    navigateToDevicePage(device);
}

EventReady.add(function(){
    initNavToDevBut();
});

var test = createElement('div', null, ['grid', 'device-container'],{key:'tabindex', value:0}, testhtml, true );
$('#testing').after(test);