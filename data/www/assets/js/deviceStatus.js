var devMode = true;

var devModeLabel = '<span class="dev-mode">Developer Mode</span>';

var deviceStatusJOBJ;

var navWebUpdate;
var navDbgCon;
var navCfg;

var navTools;
var navTools_Json;
var navTools_FileEditor;

var navCfgMqtt;
var navCfgMqtt_BinSen;
var navCfgMqtt_But;
var navCfgMqtt_Led;
var navCfgMqtt_Sen;
var navCfgMqtt_Sw;

function deviceStatusInit() {
    console.log("deviceStatusInit()");
    navWebUpdate = $('#nav_webupdate');

    navDbgCon = $('#nav_dbgCon');
    navCfg = $('#nav_cfg');

    navTools = $('#nav_tools');
    navTools_Json = $('#nav_tools_json');
    navTools_FileEditor = $('#nav_tools_fileEditor');

    navCfgMqtt = $('#nav_cfgmqtt');
    navCfgMqtt_BinSen = $('#nav_cfgmqtt_binsen');
    navCfgMqtt_But = $('#nav_cfgmqtt_but');
    navCfgMqtt_Led = $('#nav_cfgmqtt_led');
    navCfgMqtt_Sen = $('#nav_cfgmqtt_sen');
    navCfgMqtt_Sw = $('#nav_cfgmqtt_sw');

    if (devMode == false) {
        disableObj(navWebUpdate);
        disableObj(navDbgCon);
        disableObj(navCfg);

        disableObj(navTools);
        disableObj(navTools_Json);
        disableObj(navTools_FileEditor);

        disableObj(navCfgMqtt);
        disableObj(navCfgMqtt_BinSen);
        disableObj(navCfgMqtt_But);
        disableObj(navCfgMqtt_Led);
        disableObj(navCfgMqtt_Sen);
        disableObj(navCfgMqtt_Sw);

    } else {
        $('body').append(devModeLabel);
    }
    
    $('html').on('deviceStatus', setNavVis);
    getDeviceStatus(true);    
}

function getDeviceStatus(pretty) {
    var jStr = "";
    
    if(arguments.length > 0){
        var jdata = {};
        jdata["pretty"] = pretty;    
        jStr = JSON.stringify(jdata);
    }
    
    $.ajax({
        dataType: "json",
        type: 'POST',
        url: "/status",
        data: jStr.length > 0 ? jStr : null,
        success: function(data) {
            console.log("Received Device Status :");
            console.log(data);
            deviceStatusJOBJ = data;
            $('html').trigger('deviceStatus')
        },
        error: retry
    });
}

function retry(request, status, error) {

    console.log('Could not get Device Status.');
    $('#device_status').text('Could not get device status');

    //setTimeout(function(){
    //    getDeviceStatus();
    //}, 2000);

}

function setNavVis() {
    
    if ($('#device_status').length > 0) {
        $('#device_status').text(JSON.stringify(deviceStatusJOBJ));
    }
    
    var rootObj = deviceStatusJOBJ['statusObj'];

    if (rootObj === null) {
        console.log('JSON data does not contain "status" key.');
        return;
    }

    var server = rootObj['server'];

    if (server == null) {
        console.log('JSON data does not contain "server" key.');
        return;
    }

    var mqtt = rootObj['mqtt'];
    var browserObj = server['browser'];
    var servConsole = browserObj['console'];
    var cfgBrowser = browserObj['configBrowser'];
    var webUpdate = browserObj['webUpdate'];

    var tools = browserObj['tools'];


    if (tools != null) {
        var toolEnabled = false;

        var jsonVerify = tools['jsonVerify'];
        var fileEditor = tools['fileEditor'];

        if (jsonVerify != null) {
            if (jsonVerify['enabled'] == true) {
                enableObj(navTools_Json);
                toolEnabled = true;
            }
        }

        if (fileEditor != null) {
            if (fileEditor['enabled'] == true) {
                enableObj(navTools_FileEditor);
                toolEnabled = true;
            }
        }

        if (toolEnabled) {
            enableObj(navTools);
        }
    }

    if (servConsole != null) {
        if (servConsole['enabled'] == true) {
            enableObj(navDbgCon);
        }
    }

    if (webUpdate != null) {
        if (webUpdate['enabled'] == true) {
            enableObj(navWebUpdate);
        }
    }

    if (cfgBrowser != null) {
        if (cfgBrowser['enabled'] == true) {
            enableObj(navCfg);

            var hasMqttDevice = false;

            if (mqtt != null) {
                var devices = mqtt['devices'];

                if (devices['binarySensors'] > 0) {
                    enableObj(navCfgMqtt_BinSen);
                    hasMqttDevice = true;
                }

                if (devices['buttons'] > 0) {
                    enableObj(navCfgMqtt_But);
                    hasMqttDevice = true;
                }

                if (devices['lights'] > 0) {
                    enableObj(navCfgMqtt_Led);
                    hasMqttDevice = true;
                }

                if (devices['sensors'] > 0) {
                    enableObj(navCfgMqtt_Sen);
                    hasMqttDevice = true;
                }

                if (devices['switches'] > 0) {
                    enableObj(navCfgMqtt_Sw);
                    hasMqttDevice = true;
                }

                if (hasMqttDevice) {
                    enableObj(navCfgMqtt);
                }
            }
            
        }
    }
}



$(document).ready(function() {
    if ($('#device_status') == null) return;
    $('html').on('auth', deviceStatusInit);
});