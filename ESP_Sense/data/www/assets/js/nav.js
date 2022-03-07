$('.main-container').hide();

var navHtml = `<nav id="nav_head" class="navbar navbar-light navbar-expand-md navigation-clean">
    <div class="container"><a class="navbar-brand" href="/" target="_top">ESP-Sense</a><button class="navbar-toggler" data-bs-toggle="collapse" data-bs-target="#navcol-2"><span class="visually-hidden">Toggle navigation</span><span class="navbar-toggler-icon"></span></button>
        <div id="navcol-2" class="collapse navbar-collapse">
            <ul class="navbar-nav ms-auto">
                <li id="nav_cfg" class="nav-item"><a class="nav-link" href="../config.html" target="_top">Configuration</a></li>
                <li id="nav_dbgCon" class="nav-item"><a class="nav-link" href="../console.html" target="_top">Debug Console</a></li>
                <li id="nav_webupdate" class="nav-item"><a class="nav-link" href="../update.html" target="_top">Update Firmware</a></li>
                <li id="nav_tools" class="nav-item dropdown"><a class="dropdown-toggle nav-link" aria-expanded="false" data-bs-toggle="dropdown">Tools</a>
                    <div class="dropdown-menu"><a id="nav_tools_json" class="dropdown-item" href="../tools/jsonass.html" target="_top">JSON Assistant</a><a id="nav_tools_fileEditor" class="dropdown-item" href="../tools/fileeditor.html" target="_top">File Editor</a><a id="nav_tools_fileEditor-1" class="dropdown-item" href="../tools/templategen.html" target="_top">MQTT Template Generator</a></div>
                </li>
                <li id="nav_cfgmqtt-1" class="nav-item dropdown"><a class="dropdown-toggle nav-link" aria-expanded="false" data-bs-toggle="dropdown">Configure MQTT Devices</a>
                    <div class="dropdown-menu"><a id="nav_cfgmqtt-2" class="dropdown-item" href="../config/mqtt.html" target="_top">Summary</a><a id="nav_cfgmqtt_binsen" class="dropdown-item" href="#">Binary Sensors</a><a id="nav_cfgmqtt_but" class="dropdown-item" href="#">Buttons</a><a id="nav_cfgmqtt_led" class="dropdown-item" href="#">Lights</a><a id="nav_cfgmqtt_sen" class="dropdown-item" href="#">Sensors</a><a id="nav_cfgmqtt_sw" class="dropdown-item" href="#">Switches</a></div>
                </li>
            </ul><button id="logout_but" class="btn btn-primary" type="button">Log out</button>
        </div>
    </div>
</nav>`;

if($('.navbar').length == 0)
$('body').prepend(navHtml);
