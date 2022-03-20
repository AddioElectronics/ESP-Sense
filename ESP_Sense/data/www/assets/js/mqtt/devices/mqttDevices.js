class MqttDevice {
    
    static eventDeviceInfoCreated = new Set();
    
    static deviceInfo;
    static rootDevInfoKey = 'mqttDeviceInfo';
    static activeDevice;
    static stateStrings = {ok:'ok', disabled:'disabled', error:'error', unknown:'unknown', loading:'loading'};
    
    constructor(index, name, device, type, state) {
        if(arguments.length == 1){
            //index && Object.assign(this, index);
            //return;
            console.error('Can not assign MqttDevice!');
            return;
        }
        
        var dev = this;
        
        this.index = index;
        this.name = name;
        this.device = device;
        this.type = type;
        this.state = state;
        this.enabled = state == MqttDevice.stateStrings.ok;
        this.url = '/mqtt/devices/' + type + '/' + name;
        this.path = '/mqtt/devices/' + type + '/' + device;
        console.log('MqttDevice() name : ' + this.name + ' | device : ' + this.device + ' | type : ' + type + ' | state : ' + this.state + ' | url : ' + this.url)
        
        this.listElem = null;
        this.ledGroup = null;
        this.EventRequest = new Set();
        this.EventRequestComplete = new Set();
        this.EventRequestSuccess = new Set();
        this.EventRequestFailed = new Set();
        this.EventStateChanged = new Set();
        this.EventStatusChanged = new Set();
        this.EventConfigChanged = new Set();
        this.statusUpToDate = false;
        
        this.EventRequest.Add(function(){
            if(dev.ledGroup != undefined)
                dev.ledGroup.attr('state', MqttDevice.stateStrings.loading);
        });
        
        this.EventRequestComplete.Add(function(){
            if(dev.ledGroup != undefined)
                if(dev.ledGroup.attr('state') == MqttDevice.stateStrings.loading)
                    dev.ledGroup.attr('state', '');
        });
        
        this.EventRequestFailed.Add(function(){
            if(dev.ledGroup != undefined){
                dev.ledGroup.attr('state', MqttDevice.stateStrings.unknown);
            }            
        });
        
        this.EventRequestSuccess.Add(function(){
            if(dev.ledGroup != undefined)
                if(dev.ledGroup.attr('state') == MqttDevice.stateStrings.unknown)
                    dev.ledGroup.attr('state', '');          
        });
        
        this.EventStateChanged.Add(this.SetLeds);
        this.EventStatusChanged.Add(this.SetLeds);
    }

    static GetAllInfo(){
        console.log('MqttDevice.GetAllInfo()');
        
        if(devMode){
            MqttDevice.LoadDummyDeviceInfo();
            return;   
        }
        
        if(MqttDevice.UseLocalInfo() && localHasObj(MqttDevice.rootDevInfoKey)){
            let data = MqttDevice.GetLocalInfo();
            MqttDevice.ProcessAllDeviceInfo();
            //$('[action=select-device]').click(navigateToSelectedDevice);
            //MqttDevice.ResetUseLocalInfo();
        }else{
            requestJsonData('/mqtt/devices/info',
            function(data) {
                console.log('Received MQTT Device Info :');
                console.log(data);
                MqttDevice.SetInfo(data);
                MqttDevice.ProcessAllDeviceInfo();
                //MqttDevice.eventDeviceInfoCreated.Invoke();
                //$('[action=select-device]').click(navigateToSelectedDevice);
                //initNavToDevBut();
            },
            MqttDevice.RetryGetAllInfo);
        }
    }
    
    static RetryGetAllInfo(){
        console.log('MqttDevice.RetryGetAllInfo()');
        var msg = 'Failed to get MQTT Devices Info.';
        let msg2 = ' Trying again...';
       
        if(configMode){
            msg = 'Config Mode Enabled : No MQTT devices are initialized.'
            msg2 = '';
        }else{
            espAlive(function(){
            setTimeout(MqttDevice.GetAllInfo, 5000)
            },
            function(xhr, status, error){
                console.log(msg);
                $('[mqtt=devicelist] .grid.list').text(msg);
            });
        }
        
        console.log(msg + msg2);        
        $('[mqtt=devicelist] .grid.list').text(msg + msg2);
        
    }
    
    static LoadDummyDeviceInfo(){
        console.log('MqttDevice.LoadDummyDeviceInfo()');
        loadData('/assets/js/mqtt/dummy-device-list-test.json', function(data){
            MqttDevice.SetInfo(data);
            MqttDevice.ProcessAllDeviceInfo();
            //MqttDevice.eventDeviceInfoCreated.Invoke();
            //$('[action=select-device]').click(MqttDevice.NavigateToActive);
            //initNavToDevBut();
        });
    }
    
    static SetInfo(data){
        MqttDevice.deviceInfo = data;
        localStoreObj(MqttDevice.rootDevInfoKey, MqttDevice.deviceInfo);
    }
    
    static GetLocalInfo(){
        return MqttDevice.deviceInfo = localReadObj(MqttDevice.rootDevInfoKey);
    }
    
    static SetUseLocalInfo(){
        localStorage.setItem('useLocalDeviceInfo', 'true');
    }
    
    static ResetUseLocalInfo(){
        localStorage.setItem('useLocalDeviceInfo', 'false');
    }
    
    static UseLocalInfo(){
        return localStorage.getItem('useLocalDeviceInfo') == 'true';
    }
    
    static SetActiveDevice(dev, navTo){
        console.log('MqttDevice.SetActiveDevice()');
        MqttDevice.activeDevice = dev;
        mqttDevices[dev.type][dev.index] = dev;
        localStoreObj('device', dev);
        
        if(navTo)
            MqttDevice.NavigateToActive();
    }
    //(name, type, setActive)
    static GetActiveDevLocal(){
        if(!localHasObj('device')) return null;
        var device = localReadObj('device');
        return MqttDevice.activeDevice = MqttDevice.Find(device.name, device.type, true);
        //return MqttDevice.activeDevice = new MqttDevice(localReadObj('device'));
    }
    
    static NavigateToActive(){
        console.log('MqttDevice.NavigateToActive()');
        window.location.href = '/mqtt/devices/device.html';//device.info.path + '.html';
    }
    
    static ProcessAllDeviceInfo(){
        console.log('MqttDevice.ProcessAllDeviceInfo()');
        MqttDevice.AppendRawJsonData();
        MqttDevice.ProcessDeviceInfo('sensors');
        MqttDevice.ProcessDeviceInfo('binarySensors');
        MqttDevice.ProcessDeviceInfo('switches');
        MqttDevice.ProcessDeviceInfo('buttons');
        MqttDevice.ProcessDeviceInfo('lights');

        if($('[mqtt=devicelist]').length > 0){
            createDeviceList(mqttDevices);
            $('[action=select-device]').click(MqttDevice.NavigateToActive);
            initNavToDevBut();
        }
        MqttDevice.eventDeviceInfoCreated.Invoke();
    }
    
    static ProcessDeviceInfo(type){
        console.log('MqttDevice.ProcessInfo()');
        if (MqttDevice.deviceInfo[MqttDevice.rootDevInfoKey][type]) {
            var devs = MqttDevice.deviceInfo[MqttDevice.rootDevInfoKey][type];
            for (var i = 0; i < devs.length; i++) {
                var d = devs[i];
                mqttDevices[type].push(new MqttDevice(i, d['name'], d['device'], type, d['state']));
            }
        }
    }
    
    static AppendRawJsonData(){
        console.log('MqttDevice.AppendRawJsonData()');
        if ($('#info_devices').length > 0) {
            $('#info_devices').append(syntaxHighlight(JSON.stringify(MqttDevice.deviceInfo, null, 4)));
        }
    }
    
    static Find(name, type, setActive) {
        console.log('findDeviceByName() name : ' + name + ' | type : ' + type);
        var col = mqttDevices[type];
        if (col == null) return null;

        for (var i = 0; i < col.length; i++) {
            if (col[i].name == name) {
                
                if(setActive)
                    MqttDevice.SetActiveDevice(col[i]);
                
                return col[i];
            }
        }
        return null;
    }
    
    static CreateLedController(ok, disabled, error){
        let group = createElement('ledgroup' , null, 'hover-glow'/*, [{key:'name', value:dev.name}, {key:'device', value:dev.device}]*/);
        
        if(ok){
            let ledOk = createElement('led', null, null, [{key:'state', value:'ok'}], null, null, null, {key:'--led-hue', value:110});
            group.append(ledOk);
        }
        if(disabled){
            let ledOk = createElement('led', null, null, [{key:'state', value:'disabled'}], null, null, null, {key:'--led-hue', value:50});
            group.append(ledOk);
        }
        if(error){
            let ledOk = createElement('led', null, null, [{key:'state', value:'error'}], null, null, null, {key:'--led-hue', value:0});
            group.append(ledOk);
        }
        
        return group;
    }
    
    static EnableAll(subType){
        setAllMqttDevicesEnabled(true, subType);
    }
    
    static DisableAll(subType){
        setAllMqttDevicesEnabled(false, subType);
    }
    
    static SetLedState(state, group, okfn, disabledfn){
        let leds = $(group).find('led');
        leds.unbind();
        leds.removeAttr('on');
        leds.removeAttr('event');
        leds.removeAttr('disabled');
        let ledOk = $(group).find('led[state='+MqttDevice.stateStrings.ok+']');
        let ledDis = $(group).find('led[state='+MqttDevice.stateStrings.disabled+']');
        let ledErr = $(group).find('led[state='+MqttDevice.stateStrings.error+']');
        switch(state){
            case MqttDevice.stateStrings.ok: 
                ledOk.attr('on', '');
                
                if(disabledfn != null){
                    ledDis.bind('click', disabledfn);
                    ledDis.attr('event', '');   
                }
                break;
            case MqttDevice.stateStrings.error: 
                ledErr.attr('on', '');
                disableObj(ledOk);
                disableObj(ledDis);
                $(group).find('led:not([state=error]').attr('disabled', '');
                break;
            case MqttDevice.stateStrings.disabled: 
                ledDis.attr('on', '');
                
                if(okfn != null){
                    ledOk.bind('click', okfn);             
                    ledOk.attr('event', '');
                }
                break;
            case MqttDevice.stateStrings.unknown: 
                //WIP
                break;
        }
    }
    
    static GetStateFromId(id){
        switch(id){
            case '0':
            case 0: return MqttDevice.stateStrings.ok;
            case '1':
            case 1: return MqttDevice.stateStrings.disabled;
            case '2':
            case 2: return MqttDevice.stateStrings.error;
        }
        return null;
    }
    
    static SetAllDeviceLeds(){
        let dtypes  = Object.keys(mqttDevices);
        dtypes.forEach(function(element){
            let devs = mqttDevices[element];
            devs.forEach(function(dev){
                dev.SetLeds();
            });
        });
    }
    
    static SetAllLedHeadGroups(){
        console.warn('boo');
        var counts = {ok:0, disabled:0, total:0};
        var keys  = Object.keys(mqttDevices);
        keys.forEach(function(key){
            if($('ledgroup[device-type='+key+']').length == 0) return;
            let subcounts = MqttDevice.SetSubTypeLeds(key);
            for(let ckey in counts){
                counts[ckey] += subcounts[ckey];
            }
        });
        
        let group = $('ledgroup[device-type=all]');
        
        MqttDevice.SetLedHeadGroup(group, counts);
        
    }
    
    static SetSubTypeLeds(subType){
        let group = $('ledgroup[device-type='+subType+']');    
        
        if(group.length == 0) return;
        
        function ledQuery(state) {return '.device-group[device-type='+subType+'] .sub-list led[state='+state+']'};
        
        return MqttDevice.SetLedHeadGroup(group, ledQuery);
        
        let okLen = $(ledQuery('ok') + '[on]').length;
        let disLen = $(ledQuery('disabled') + '[on]').length;
        let errLen = $(ledQuery('error')+'[on]').length;
        let totalLen = $(ledQuery('error')).length - errLen;
        
        removeAttr(group, 'on');
        removeAttr(group, 'event');
        group.off();
        
        let disLed = group.parent().find('led[state=disabled]');  
        let okLed = group.parent().find('led[state=ok]');
        
        if(okLen == totalLen)
            okLed.attr('on', '');
        
        if(okLen != totalLen){
            okLed.attr('event', '');  
            okLed.on('click', function(){
                MqttDevice.EnableAll(subType);
            });
        }
        
         if(disLen == totalLen)
            disLed.attr('on', '');  
        
        if(disLen != totalLen){
            disLed.attr('event', '');  
            disLed.on('click', function(){
                MqttDevice.DisableAll(subType);
            });
        }
        console.log('tired, reminder to redo');
        
        return {ok:okLen, disabled:disLen, total:totalLen};
    }
    
    //Do not call, used by
    static SetLedHeadGroup(group, queryfn){
        
        let counts = queryfn;
        if(queryfn instanceof Function){
            let okLen = $(queryfn('ok') + '[on]').length;
            let disLen = $(queryfn('disabled') + '[on]').length;
            let errLen = $(queryfn('error')+'[on]').length;
            let totalLen = $(queryfn('error')).length - errLen;
            
            counts = {ok:okLen, disabled:disLen, total:totalLen};
        }
        
        var groupLeds = group.find('led');
        
        removeAttr(groupLeds, 'on');
        removeAttr(groupLeds, 'event');
        groupLeds.off();
        
        let disLed = group.find('led[state=disabled]');  
        let okLed = group.find('led[state=ok]');
        
        if(counts.ok == counts.total)
            okLed.attr('on', '');
        
        if(counts.ok != counts.total){
            okLed.attr('event', '');  
            var dtype = group.attr('device-type');
            okLed.on('click', function(){
                MqttDevice.EnableAll(dtype);
            });
        }
        
         if(counts.disabled == counts.total)
            disLed.attr('on', '');  
        
        if(counts.disabled != counts.total){
            disLed.attr('event', '');  
            var dtype = group.attr('device-type');
            disLed.on('click', function(){
                MqttDevice.DisableAll(dtype);
            });
        }
        console.log('tired, reminder to redo');
        
        return counts;
    }
    
    
    
}

async function setAllMqttDevicesEnabled(enabled, subType, dontSetAll){
    
    if(subType != null && subType != 'all'){
        let arr = mqttDevices[subType];
        
        if(arr == undefined || arr.length == 0){
            console.log('Could not SetEnabled to '+enabled+' for all ' + subType + '. Device Type does not exist or empty.')
            return false;
        }
        
        for(let i = 0; i < arr.length; i++){
            let dev = arr[i];
            await dev.SetEnabled(enabled)
        }
        
        //MqttDevice.SetSubTypeLeds(subType);
        if(!dontSetAll)
        MqttDevice.SetAllLedHeadGroups();
        return;
    }
    
    for(var key in mqttDevices){
        await setAllMqttDevicesEnabled(enabled, key, true);
    }
    MqttDevice.SetAllLedHeadGroups();
}


MqttDevice.prototype.UrlStatus = function(){return this.url + '/status'}
MqttDevice.prototype.UrlState = function(){return this.url + '/state'}
MqttDevice.prototype.UrlConfig = function(){return this.url + '/config'}
MqttDevice.prototype.UrlSetEnabled = function(){return this.url + '/setEnabled'}
MqttDevice.prototype.IsImportant = function(){
    if(this.config == undefined) return undefined;
    return this.config[this.name]['deviceConfig']['important'];
}


MqttDevice.prototype.GetStatus = async function(success, errorcb, complete){
    var dev = this;
    this.EventRequest.Invoke();
    await requestJsonData(this.UrlStatus(), function(data){
        dev.status = JSON.parse(data);
        dev.state = GetStateFromId(dev.status['deviceStatus']['state']);
        dev.EventStatusChanged.Invoke();
        dev.statusUpToDate = true;
        if(success != null)
            success(dev, data);
        
        dev.EventRequestSuccess.Invoke();
    }, function(xhr, status, error){
        dev.status = undefined;
        dev.statusUpToDate = false;
        if(errorcb != null)
            errorcb(xhr, status, error);
        
        dev.EventRequestFailed.Invoke();
    }, function(){
        if(complete != null)
            complete();
        dev.EventRequestComplete.Invoke();
    });
}

MqttDevice.prototype.GetState = async function(success, errorcb, complete){
    var dev = this;
    this.EventRequest.Invoke();
    await requestJsonData(this.UrlState(), function(data){
        dev.state = JSON.parse(data)['state'];
        dev.EventStateChanged.Invoke();        
        
        if(success != null)
            success(data);
        dev.EventRequestSuccess.Invoke();
    }, function(xhr, status, error){
        dev.status = undefined;
        if(errorcb != null)
            errorcb(xhr, status, error);
        dev.EventRequestFailed.Invoke();
    }, function(){
        if(complete != null)
            complete();
        dev.EventRequestComplete.Invoke();
    });
}

MqttDevice.prototype.GetConfig = async function(success, errorcb, complete){
    var dev = this;
    this.EventRequest.Invoke();
    await requestJsonData(this.UrlConfig(), function(data){
        dev.config = JSON.parse(data);
        dev.EventConfigChanged.Invoke();
        if(success != null)
            success(data);
        dev.EventRequestSuccess.Invoke();
    }, function(xhr, status, error){
        dev.config = null;
        dev.EventConfigChanged.Invoke();
        if(errorcb != null)
            errorcb(xhr, status, error);
        dev.EventRequestFailed.Invoke();
    }, function(){
        if(complete != null)
            complete();
        dev.EventRequestComplete.Invoke();
    });
}

MqttDevice.prototype.SetConfig = async function(data, success, error){
    console.warn(this);
    console.log(data);
    alert('MqttDevice.prototype.SetConfig');
    error();
}

MqttDevice.prototype.SetEnabled = async function(enabled, success, error, complete){
    var dev = this;
    
    if(enabled && dev.state != MqttDevice.stateStrings.disabled) return;
    if(!enabled && dev.state != MqttDevice.stateStrings.ok) return;
    
    if(devMode){
        if(enabled){
            this.state = 'ok';
        }else{
            this.state = 'disabled';
        }
        this.EventStateChanged.Invoke();
    }else{
        this.EventRequest.Invoke();
         $.ajax({
            type: 'POST',
            url: dev.UrlSetEnabled(),
            data: null,
            success: function(data){
                console.log(dev.name + ' SetEnabled('+enabled+') Success');
                dev.state = JSON.parse(data)['state'];
                dev.statusUpToDate = false;
                dev.SetLeds();
                dev.EventStateChanged.Invoke();
                dev.EventRequestSuccess.Invoke();
            },
            error: function(xhr, status, error){
                console.log(dev.name + ' SetEnabled('+enabled+') Error : ' + xhr.responseText);
                dev.state = MqttDevice.stateStrings.unknown;
                dev.statusUpToDate = false;
                dev.EventRequestFailed.Invoke();
            },
            complete: function(){
                if(complete != null)
                    complete();
                dev.EventRequestComplete.Invoke();
            }
        });
    }
    
    

    this.SetLeds();
}


MqttDevice.prototype.SetLeds = function(){
    function ledClickEvent(e){
        
        let ledElem = $(e.target);
        let devElem = $($(e.target).parent());
        let index = devElem.attr('dev-index');
        let type = devElem.attr('dev-type');
        let dev = mqttDevices[type][index];
        dev.SetEnabled(ledElem.attr('state') == MqttDevice.stateStrings.ok);
    }
        
    if(this.ledGroup != null)
        MqttDevice.SetLedState(this.state, this.ledGroup, ledClickEvent, ledClickEvent)
    
    MqttDevice.SetSubTypeLeds(this.type);
}


MqttDevice.eventDeviceInfoCreated.add(MqttDevice.SetAllDeviceLeds);

onEvent('auth', function(){
    if ($('[mqtt]') == null) return;
    MqttDevice.eventDeviceInfoCreated.add(MqttDevice.GetActiveDevLocal);
    MqttDevice.GetAllInfo();
});