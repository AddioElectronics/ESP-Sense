var rstHtml = `<button id="esp_reset_button" class="btn btn-primary" type="button">Reset ESP</button>`;

var rstBut;

function espControllerInit() {
    console.log('espControllerInit');
    if($('#esp_reset_button').length == 0){
        $('body').append(rstHtml);
    }
    Show(rstBut);
    $('html').on('auth', function() {
        enableObj(rstBut);
    });
    rstBut.click(espReset);
}

function espReset() {
    if (updateInProgess || !espAuthenticated) return;
    console.log('Sending Reset Request...');
    $.ajax({
        type: 'POST',
        url: "/reset",
        data: null,
        success: espIsResetting,
        error: function(a, b, c) {
            console.log('Reset Failed');
        }
    });
}

function refreshOnResponse(){
    setTimeout(function(){
            $.ajax({
            type: 'GET',
            url: "/alive",
            data: null,
            success: refreshPage,
            error:function(request, status, error){
                refreshOnResponse();
            }
        });
    }, 5000);
}

function espIsResetting(){
    console.log('Resetting ESP');
    enableBlockingMsg('Resetting ESP...');
    refreshOnResponse();
}

$(document).ready(function() {
    rstBut = $('#esp_reset_button');
    console.log(rstBut);
    rstBut.hide();
    $('html').on('resetting', espIsResetting);
    espControllerInit();
});