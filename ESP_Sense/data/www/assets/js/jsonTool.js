function jsonToolInit(){
    Show($('#tool_parent'));
    $('iframe').contents().find('.page-header').remove();
    
}


$(document).ready(function(){
    $("#iframeID").ready(function () { //The function below executes once the iframe has finished loading
         jsonToolInit();
    });
   
});