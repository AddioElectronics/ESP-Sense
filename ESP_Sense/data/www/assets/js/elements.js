//Build an element with multiple function calls.
//After call Create()
class ElementBuilder {
    constructor(type, id, classes, attrs, text, isHtml, events, css) {
        this.type = type;
        this.id = id != null ? id : null;
        this.classes = classes != null ? classes : null;
        this.attrs = attrs != null ? attrs : null;
        this.text = text != null ? text : null;
        this.isHtml = isHtml != null ? isHtml : null;
        this.events = events != null ? events : null;
        this.css = css;
    }
    
    //Pass an array or single value
    static AddValue(dest, values){
        if(dest == null){
            dest = values;
        }else{
            if(Array.isArray(dest)){
                if(Array.isArray(values))
                    dest.concat(values);
                else
                    dest.push(values);
            }else{
                dest = [dest];
                ElementBuilder.AddValue(dest, values);
            }
        }
        return dest;
    }
    
}


    ElementBuilder.prototype.Id = function(id){
        this.id = id;
    }
    
    //Add either array of strings or single string
    ElementBuilder.prototype.Classes = function(classes){
        this.classes = ElementBuilder.AddValue(this.classes, classes);
        return;
        if(this.classes == null){
            this.classes = classes;
        }else{
            if(Array.isArray(this.classes)){
                if(Array.isArray(classes))
                    this.classes.concat(classes);
                else
                    this.classes.push(classes);
            }else{
                elem.addClass(this.classes);
            }
        }
    }
    
    //Pass either 1 object (or array) to attrsKey [{key:'key', value:'value'}] or
    //pass key to attrsKey and value to value
    ElementBuilder.prototype.Attrs = function(attrsKey, value){
        if(arguments.length > 1)
            attrsKey = {key:attrsKey, value:value};
        
        this.attrs = ElementBuilder.AddValue(this.attrs, attrsKey);
        return;
        this.attrs = attrs;
    }

    //Pass either 1 object (or array) to cssKey [{key:'key', value:'value'}] or
    //pass key to cssKey and value to value
    ElementBuilder.prototype.Css = function(cssKey, value){
        if(arguments.length > 1)
            cssKey = {key:cssKey, value:value};
        this.css = ElementBuilder.AddValue(this.css, cssKey);
        return;
        this.css = css;
    }
    
    ElementBuilder.prototype.Text = function(text){
        this.isHtml = false;
        this.text = text;        
    }
    
    ElementBuilder.prototype.Html = function(html){
        this.text = html;
        this.isHtml = true;
    }
    
    ElementBuilder.prototype.AddEvent = function(name, fn){
        if(this.events == null)
            this.events = {};
        
        if(this.events[name] == null){
            this.events[name] = [];
        }
        this.events[name].push(fn);
    }
    
    ElementBuilder.prototype.Click = function(fn){
        this.AddEvent('click', fn);
    }
    
    ElementBuilder.prototype.Create = function(){
        var elem = document.createElement(this.type);
        elem = $(elem);
    
        if(this.id != null)
        elem.attr('id', this.id);
    
        if(this.classes != null){
            if(Array.isArray(this.classes)){
                for(var i = 0; i < this.classes.length; i++){
                    elem.addClass(this.classes[i]);
            }
            }else{
                elem.addClass(this.classes);
            }
        }
    
        if(this.attrs != null){
            if(Array.isArray(this.attrs)){
                for(var i = 0; i < this.attrs.length; i++){
                    addAttr(elem, this.attrs[i]);
                }
            }else{
                addAttr(elem, this.attrs);
            }
        }
    
        if(this.css != null){
            if(Array.isArray(this.css)){
                for(var i = 0; i < this.css.length; i++){
                    elem.css(this.css[i].key, this.css[i].value);
                }
            }else{
                elem.css(this.css.key, this.css.value);
            }
        }
    
        if(this.text != null && this.text.length > 0){
            if(this.isHtml == undefined || this.isHtml == false){
                elem.text(this.text);
            }else{
                elem.html(this.text);
            }
        }
        
        if(this.events != null)
        for (let key in this.events) {           
            for(let i = 0; i < this.events[key].length; i++){
                 $(elem).on(key, this.events[key][i]);
            }
        }

        return elem;
        
        var elem = createElement(this.type, this.id, this.classes, this.attrs, this.text, this.isHtml, null, this.css);
        
        for (let key in this.events) {           
            for(let i = 0; i < this.events[key].length; i++){
                 $(elem).on(key, this.events[key][i]);
            }
        }
        
        return elem;
    }

//Quickly create an element with 1 function call.
function createElement(type, id, classes, attrs, text, isHtml, eClick, css){
    var elem = new ElementBuilder(type, id, classes, attrs, text, isHtml, (eClick != null ? {key:'click', value:eClick} : null), css );
    
    return elem.Create();
}

$('.remove-me').remove();

var classTableNoBorders = 'table-borderless';

function clickToggleBorder(){
    var id = $(this).attr('border-id');
    var tables = id == undefined || id.length == 0  ? $(this).parent().find('table') : $('[border-id='+id+']').find('table');
    var noBorders = tables.hasClass(classTableNoBorders);
    console.log('has borders ' + noBorders);
    
    if(noBorders)
        tables.removeClass(classTableNoBorders)
    else
        tables.addClass(classTableNoBorders);
        
}

$('button[action=border-toggle]').click(clickToggleBorder);

function clickHideContent(){
    var id = $(this).attr('hide-id');
    var content = id == undefined || id.length == 0 ?  $(this).next() : $('[hide-id='+id+']:not(button)');
    var vis = $(content).is(":visible");
    console.log('is visible ' + vis);
    
    if(vis){
        content.hide();
        $(this).text('Show');
    }else{
        content.show();
        $(this).text('Hide');
    }      
}

$('button[action=hide-content]').click(clickHideContent);




