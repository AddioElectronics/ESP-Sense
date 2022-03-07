var blockingMessage = `<h1 id="blocking_message" class="fadein">Heading</h1>`;
var blockingOverlay = `<div id="blocking_overlay" class="fadein"></div>`;

function Show(obj) {
    $(obj).show();
    $(obj).removeClass('d-none');
}

function disableObj(obj) {
    $(obj).attr('disabled','disabled');
}

function enableObj(obj) {
    $(obj).removeAttr('disabled');
}

function enableBlockingMsg(msg) {
    
    $('body').append(blockingMessage);
    $('body').append(blockingOverlay);

    $('#blocking_message').text(msg);
}

function disableBlockingMsg() {
    $('#blocking_message').remove();
    $('#blocking_overlay').remove();
}

function onAuth(fn) {
    $('html').on('auth', fn);
}

function refreshPage() {
    window.location.reload();
    window.location.href = window.location.href;
}

function syntaxHighlight(json) {
    if (typeof json != 'string') {
         json = JSON.stringify(json, undefined, 2);
    }
    json = json.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
    return json.replace(/("(\\u[a-zA-Z0-9]{4}|\\[^u]|[^\\"])*"(\s*:)?|\b(true|false|null)\b|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?)/g, function (match) {
        var cls = 'number';
        if (/^"/.test(match)) {
            if (/:$/.test(match)) {
                cls = 'key';
            } else {
                cls = 'string';
            }
        } else if (/true|false/.test(match)) {
            cls = 'boolean';
        } else if (/null/.test(match)) {
            cls = 'null';
        }
        return '<span class="' + cls + '">' + match + '</span>';
    });
}

