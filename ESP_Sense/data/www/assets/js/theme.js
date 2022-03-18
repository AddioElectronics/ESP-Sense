var pageTheme = 'fixed';

if(pageTheme != null){
    $('html').attr('theme', pageTheme);
}

EventReady.Add(function(){
    $('nav .gradient').appendTo($('nav'));    
});