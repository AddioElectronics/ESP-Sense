var pageTheme = 'fixed';

if(pageTheme != null){
    $('html').attr('theme', pageTheme);
}

EventReady.Add(function(){
    
    switch(pageTheme){
        case 'darkneon':
        case 'darkspooky':
            $('nav .gradient').appendTo($('nav'));
            break;
    }
    
});