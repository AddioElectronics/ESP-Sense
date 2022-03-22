var updateInProgess = false;

/*
Authentication currently just hides and shows the correct forms, 
in the future it will only serve pages after authentication, else serve login page.
Not a security risk, if someone un-hid the form and tried to upload the firmware would not allow it.
*/

//Initialize the Updater form
function webupdate_init() {
    console.log("webupdate_init()");
    Show($('#update_con'));
    $('#update_progress').hide();
    $('#debug_output').hide();
    $('#file_input').on('change', function() {
        webupdate_filechanged($(this));
    });

    //$('#upload_button').on('click', webupdate_submit);
    //$('#upload_button').on('submit', webupdate_submit);

    $('#update_form').submit(function(e) {
        
        //Check if still authenticated.
        //Without this progress bar will reach 100% and no update will happen.
        Authenticator.Start(function(){
            console.log("Still authenticated.");
        }, function(){
            refreshPage();
        });        
        webupdate_submit();
        e.preventDefault();
        var form = $('#update_form')[0];
        var data = new FormData(form);
        $.ajax({
            url: '/update/upload',
            type: 'POST',
            data: data,
            contentType: false,
            processData: false,
            xhr: function() {
                updateInProgess = true;
                disableObj($('#esp_reset_button'));
                var xhr = new window.XMLHttpRequest();
                xhr.upload.addEventListener('progress', function(evt) {
                    if (evt.lengthComputable) {
                        var per = ((evt.loaded / evt.total) * 100).toFixed(2);
                        addToConsole("Progress : " + per);
                        setProgress(per);
                        if(per == 100){
                            setUploadMessage('Update Complete! Device is restarting...', false);
                            setTimeout(function(){
                                invokeEvent('resetting');
                            }, 3500);                            
                        }
                    }
                }, false);
                return xhr;
            },
            success: function(d, s) {
                addToConsole("Success!");
            },
            error: function(a, b, c) {
                updateInProgess = false;
                enableObj($('#esp_reset_button'));
            }
        });
    });
}

function webupdate_submit() {
    addToConsole('Upload started...');

    var submitButton = $('#update_button');
    setUploadBtnState(false);

    setUploadMessage('Uploading... Please be patient.', true);
    Show($('#update_progress'));
    //Show($('#debug_output'));
}

function webupdate_filechanged(input) {
    if (input.val() == '') return;

    var filename = input.val().split('\\').pop();

    if (filename.split('.').pop() != 'bin') {
        setUploadMessage('Firmware file must be a binary!', true);
        //alert("Firmware file must be a binary!");
        input.val('');
        setUploadBtnState(false);
    } else {
        setUploadMessage('File OK! Ready to upload.', false);
        setUploadBtnState(true);
    }
}

function setUploadBtnState(en) {
    var btn = $('#upload_button');
    if (en) {
        btn.removeClass('disabled');
        btn.removeAttr('disabled');
    } else {
        btn.addClass('disabled');
        btn.attr('disabled');
    }
}

function setUploadMessage(msg, err) {
    var mobj = $('#error_message');
    if (err) {
        mobj.addClass('error');
        mobj.removeClass('message');
    } else {
        mobj.removeClass('error');
        mobj.addClass('message');
    }

    mobj.text(msg);
}

function setProgress(prog){
    Show($('#update_progress'));
    Show($('#update_progress').children());
    $('#update_progress').children().attr('aria-valuenow', prog);
    $('#update_progress').children().width(prog + '%');
    $('#update_progress').children().text(prog + '%');
}

function addToConsole(msg) {
    console.log(msg);
    $('#debug_output').append('<p>' + msg + '</p>');
}

EventReady.Add(function() {
    if($('#update_form').length == 0) return;
    onEvent('auth', webupdate_init);
});