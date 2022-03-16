//Contains functions for creating tables for displaying status data

function createStatusTable(name, caption, capTop){

    let cap = null;
    if(caption != null){
        cap = new Element('caption');
        cap.Text(caption);
        if(capTop != null)
            cap.Classes('caption-top');
    }
    
    
    let tabRoot = new ElementBuilder('div');
    tabRoot.Classes('font-monospace'); 
    
    let table = new ElementBuilder('table');
    table.Classes(['table', 'table-sm']);
    table.Attrs({key:'status-content', value:name})
    
    
    let tbody = new ElementBuilder('tbody');
    
    var tabRootElem = tabRoot.Create();
    let tableElem = table.Create();
    let tbodyElem = tbody.Create();    
    
    if(cap != null)
        tableElem.append(cap.Create());
    
    tableElem.append(tbodyElem);
    tabRootElem.append(tableElem);
    
    return $(tabRootElem);
}

function populateStatusTable(elem, jdata, caption){
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
            let subtable = createStatusTable(key, caption ? key : null, true);
            val = createElement('td');
            val.append(subtable);
            populateStatusTable(subtable.find('tbody'), jdata[key], caption);
        }else{
            getValueTypeClass(value)
            val = createElement('td', null, type, null, value instanceof String ? value : value.toString());   
            
        }
        
        $(row).append(name);
        $(row).append(val);
        $(elem).append(row);
    }
}

function objectToTables(parent, obj, caption){
    for(let key in obj){
        let elem = createStatusTable(key, caption ? key : null, true);
        $(elem.find('table')[0]).addClass('table-tree');
        populateStatusTable(elem.find('tbody'), obj[key], caption);
        $(parent).append(elem);
    }
}
