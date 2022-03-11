var mqttDevices = {sensors:[], binarySensors:[], lights:[], buttons:[], switches:[]}

var espVersion = [0,0,0];

function findMqttDevice(name, type) {
    console.log('findDeviceByName() name : ' + name + ' | type : ' + type);
    var col = mqttDevices[type];
    if (col == null) return null;
   
    for (var i = 0; i < col.length; i++) {
        if (col[i].name == name) {
            return {
                index: i,
                info: col[i]
            };
        }
    }
}