$('main').hide();

var navHtml = `<nav id="nav_head" class="navbar navbar-light navbar-expand-md navigation-clean">
    <div class="container"><a class="navbar-brand" href="/" target="_top">ESP-Sense</a><button class="navbar-toggler" data-bs-toggle="collapse" data-bs-target="#navcol"><span class="visually-hidden">Toggle navigation</span><span class="navbar-toggler-icon"></span></button>
        <div id="navcol" class="collapse navbar-collapse">
            <ul class="navbar-nav ms-auto">
                <li id="nav_cfg" class="nav-item"><a class="nav-link" href="/config.html" target="_top">Configuration</a></li>
                <li id="nav_dbgCon" class="nav-item"><a class="nav-link" href="/console.html" target="_top">Debug Console</a></li>
                <li id="nav_webupdate" class="nav-item"><a class="nav-link" href="/update.html" target="_top">Update Firmware</a></li>
                <li id="nav_cfgmqtt" class="nav-item"><a class="nav-link" href="/mqtt/devices.html" target="_top">MQTT Devices</a></li>
                <li id="nav_tools" class="nav-item dropdown"><a class="dropdown-toggle nav-link" aria-expanded="false" data-bs-toggle="dropdown">Tools</a>
                    <div class="dropdown-menu"><a id="nav_tools_json" class="dropdown-item" href="/tools/jsonass.html" target="_top">JSON Assistant</a><a id="nav_tools_fileEditor" class="dropdown-item" href="/tools/fileeditor.html" target="_top">File Editor</a><a id="nav_tools_templateGen" class="dropdown-item" href="/tools/templategen.html" target="_top">MQTT Template Generator</a></div>
                </li>
            </ul><button id="logout_but" class="btn btn-primary" type="button">Log out</button>
        </div>
    </div>
</nav>`;

var footerHtml = `<footer>
    <div class="container">
        <div class="grid">
            <h3 class="footing">ESP Sense</h3>
            <div class="github"><a href="https://github.com/AddioElectronics/ESP-Sense"><i class="fa fa-github"></i></a></div><span class="version">Version x.x.x</span>
        </div>
    </div>
</footer>`;

if($('.navbar').length == 0)
$('body').prepend(navHtml);

if($('footer').length == 0)
$('body').append(footerHtml);

function setMainHeight(){
    let navHeight = $('nav').height();
    let footerHeight = $('footer').height();
    let windowHeight = $('html').height();
    let mainHeight = windowHeight - navHeight - footerHeight;
    //$('main').height(mainHeight);
}

EventWindowResize.add(setMainHeight);
setMainHeight();

function moveFooter(){
    
}

EventWindowScroll.add(setMainHeight);
