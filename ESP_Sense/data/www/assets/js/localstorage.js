function localStoreObj(name, value){
    var jstr = JSON.stringify(value);
    localStorage.setItem(name, jstr);
}

function localReadObj(name){
    let obj = localStorage.getItem(name);
    if(obj == undefined) return undefined;
    if(obj == null) return null;
    return JSON.parse(obj);
}
