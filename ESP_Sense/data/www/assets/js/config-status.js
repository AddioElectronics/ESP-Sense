function createStatusTable(name, caption, capTop){

    let cap = null;
    if(caption != null){
        cap = new Element('caption');
        head.Text(caption);
        if(capTop != null)
            cap.Classes('caption-top');
    }
    
    
    let tabRoot = new Element('div');
    tabRoot.Classes('font-monospace'); 
    
    let table = new Element('table');
    table.Classes(['table', 'table-sm']);
    table.Attrs({key:'status-content', value:name})
    
    
    let tbody = new Element('tbody');
    let tbodyElem = tbody.Create();    
    
    if(cap != null)
        table.append(cap.Create());
    
    table.append(tbodyElem);
    
    let tableElem = table.Create();
    tabRoot.append(tableElem);
    
    var tabRootElem = tabRoot.Create();
    
    return tabRootElem;
}

function populateStatusTable(elem, jdata){
    console.log('createDeviceStatus');
    console.log(jdata);
    elem.find('tr').remove();
    
    for (let key in jdata) {
        let value = jdata[key];
        let type = getValueTypeClass(value);
        let row = createElement('tr');
        let name = createElement('td', null, null, null, key);
        let val;
        
        if(value instanceof Object){
            let subtable = createStatusTable(key);
            val = createElement('td');
            populateStatusTable(key, val);
        }else{
            val = createElement('td', null, type, null, value instanceof String ? value : value.toString());            
        }
          
        $(row).append(name);
        $(row).append(val);
        elem.append(row);
    }
}

function objectToTables(parent, obj){
    
    for(let key in obj){
        let elem = createStatusTable(key);
        $(parent).append(elem);
    }
}

function loadDummyGlobalStatus(){
    loadData('/assets/js/dummyStatus.json', function(data){
        let statusObj = data['status'];
        let retainedObj = data['retainedStatus'];
        let dsobj = $('#global-status');
        
        objectToTables(gsobj, root)

    });
}

function loadDummyConfigStatus(){
    loadData('/assets/js/mqtt/dummy-status-test.json', function(data){
        let rootObj = data['co2'];
        let dsobj = $('.device-status');
        populateStatusTable(dsobj.find('[status-content=device]').find('tbody'), rootObj['deviceStatus']);
        populateStatusTable(dsobj.find('[status-content=deviceType]').find('tbody'), rootObj['sensorStatus']);
        populateStatusTable(dsobj.find('[status-content=unique]').find('tbody'), rootObj['uniqueStatus']);
        populateStatusTable(dsobj.find('[status-content=website]').find('tbody'), rootObj['websiteStatus']);
    });
}


EventReady.add(function(){
    if ($('[mqtt]') == null) return;
    onEvent('auth', devicesInfoInit);
    
     if(devMode){
         loadDummyConfigStatus();
            loadDummyGlobalStatus();   
     }
});