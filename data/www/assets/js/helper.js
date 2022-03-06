var blockingMessage = `<h1 id="blocking_message"></h1>`;
var blockingOverlay = `<div id="blocking_overlay"></div>`;

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



