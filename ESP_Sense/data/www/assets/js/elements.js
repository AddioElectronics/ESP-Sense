class Element {
    constructor(type, id, classes, attrs, text, isHtml) {
        this.type = type;
        this.id = id != null ? id : null;
        this.classes = classes != null ? classes : null;
        this.attrs = attrs != null ? attrs : null;
        this.text = text != null ? text : null;
        this.isHtml = isHtml != null ? isHtml : null;
        this.events = {};
    }
}

    Element.prototype.Id = function(id){
        this.id = id;
    }
    
    Element.prototype.Classes = function(classes){
        this.classes = classes;
    }
    
    Element.prototype.Attrs = function(attrs){
        this.attrs = attrs;
    }
    
    Element.prototype.Text = function(text){
        this.text = text;
        this.isHtml = false;
    }
    
    Element.prototype.Html = function(html){
        this.text = html;
        this.isHtml = true;
    }
    
    Element.prototype.AddEvent = function(name, fn){
        if(this.events[name] == null){
            this.events[name] = [];
        }
        this.events[name].push(fn);
    }
    
    Element.prototype.Click = function(fn){
        this.AddEvent('click', fn);
    }
    
    Element.prototype.Create = function(){
        var elem = createElement(this.type, this.id, this.classes, this.attrs, (this.isHtml ? null : this.text), (isHtml ? this.text : null), this.isHtml);
        
        for (let key in this.events) {           
            for(let i = 0; i < this.events[key].length; i++){
                 $(elem).on(key, this.events[key][i]);
            }
        }
        
        return elem;
    }


var classTableNoBorders = 'table-borderless';

$('button[action=border-toggle]').click(function(){
    var id = $(this).attr('actionid');
    var tables = $($(this).parent().find((id.length > 0 ? 'table[actionid='+id+']' : 'table') ));
    var noBorders = tables.hasClass(classTableNoBorders);
    console.log('has borders ' + noBorders);
    
    if(noBorders)
        tables.removeClass(classTableNoBorders)
    else
        tables.addClass(classTableNoBorders);
        
});

$('button[action=hide-content]').click(function(){
    var id = $(this).attr('actionid');
    var content = id.length > 0 ? $(this).parent().find('[action=hide-content][actionid='+id+']:not(button)') : $(this).next();
    var vis = $(content).is(":visible");
    console.log('is visible ' + vis);
    
    if(vis){
        content.hide();
        $(this).text('Show');
    }else{
        content.show();
        $(this).text('Hide');
    }
        
        
});