var themeDefault = 'Dark'; //Default
var themeDefaultHue = 208;

var themeElements = `<div id="theme_container"><select id="theme_select" name="Fixed" value="fixed">
        <optgroup label="Theme">
            <option value="dark" selected>Dark</option>
            <option value="fixed">Dark Fixed</option>
            <option value="darkspooky">Dark Spooky</option>
            <option value="darkneon">Dark Neon</option>
        </optgroup>
    </select><input id="theme_hue" class="form-range custom" type="range" min="0" max="360" step="1" style="--slider-thumb-hue: 0 !important;" /></div>`;

var themeSelect;
var themeSliderHue;

var themeObj = {theme:themeDefault, hue:208};

function changeTheme(theme){
    $('html').attr('theme', theme);
    themeObj.theme = theme; 
    localStoreObj('theme', themeObj);
        
    if(theme == 'darkneon'){
        Show(themeSliderHue);
    }else{
        themeSliderHue.hide();
    }
}

function changeThemeHue(hue){
    themeObj.hue = hue;
    localStoreObj('theme', themeObj);
    $('html').cssImportant('--theme-hue', hue);
    themeSliderHue.cssImportant('--slider-thumb-hue', hue);  
}

function themeHueChanging(){
    let hue = themeSliderHue.val();
    changeThemeHue(hue);
}

EventReady.Add(function(){
    $('nav .gradient').appendTo($('nav'));
    
    $('body').append(themeElements);
    
    themeSelect = $('#theme_select');
    themeSliderHue = $('#theme_hue'); 
    
    themeSliderHue.hide();
    
    if(localHasObj('theme')){
        themeObj = localReadObj('theme');
        changeTheme(themeObj.theme);
        themeSelect.val(themeObj.theme);
        themeSliderHue.val(themeObj.hue);
        changeThemeHue(themeObj.hue);
    }else if(themeDefault != null){
        changeTheme(themeDefault);
        themeSelect.val(themeDefault);
        themeSliderHue.val(themeDefaultHue);
        changeThemeHue(themeDefaultHue);
    }
    
    themeSelect.change(function(){
        let value = $(this).val();
        changeTheme(value);
    });
    
    
    
    themeSliderHue.mouseenter(function(){
        EventMouseMove.Add(themeHueChanging);  
    });
    
    themeSliderHue.mouseleave(function(){
        EventMouseMove.delete(themeHueChanging);  
    });
});