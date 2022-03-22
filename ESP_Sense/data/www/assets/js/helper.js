//Show an element
//Also removes the bootstrap d-none class.
function Show(obj) {
    $(obj).show();
    $(obj).removeClass('d-none');
}

//Add an attribute with or without a key, or an array of attributes.
//Pass either string, object{key:,value:}, or array.
function addAttr(obj, attr){    
    function addToObj(obj, attr){
        if(attr.value != null){
            $(obj).attr(attr.key, attr.value);
        }else{
            if(attr.key != null){
                $(obj).attr(attr.key, '');
            }else{
                $(obj).attr(attr, '');  
            }        
        }
    }
    
    if(Array.isArray(obj)){
        obj.forEach(elem => addToObj(elem, attr));
    }else{
       addToObj(obj, attr); 
    }
}

//Remove an attribute from 1 or an array of elements.
function removeAttr(obj, attr){    
    if($.isArray(obj)){
        obj.forEach(elem => $(elem).removeAttr(attr));
    }else{
        $(obj).removeAttr(attr)
    }
}

//Disables an element.
//param obj         : element or jquery array of elements to disable.
//param markAlready : If object is already disabled, add an attribute which specifies enableObj to leave it disabled.
function disableObj(obj, markAlready) {
    obj = $(obj);
    if(obj.length > 1){
        obj.each(function(index){
           disableObj(obj[index], markAlready); 
        });
        return;
    }
    
    if(obj.attr('disabled') != undefined && markAlready){
        addAttr(obj, 'keep-disabled');
    }
    
    addAttr(obj, 'disabled');
    obj.addClass('disabled');
}

//Disables an element.
//param obj             : element or jquery array of elements to disable.
//param ignoreAlready   : Ignore the keep-disabled attribute to force enabling.
function enableObj(obj, ignoreAlready) {
    obj = $(obj);
    if(obj.length > 1){
       obj.each(function(index){
           enableObj(obj[index], ignoreAlready); 
        });
        return;
    }
    
    if(obj.attr('keep-disabled') == undefined || ignoreAlready == true){
        removeAttr(obj, 'disabled');
        obj.removeClass('disabled');
    }   
    removeAttr(obj, 'keep-disabled');
}

//Displays a message over the middle of the screen blocking the page.
//param blockNav    : Overlay the navigation so it can not be used.
//param time        : How long to display the message for
//param textElem    : Type of text element (h1, span, p)
function enableBlockingMsg(msg, blockNav, color, time, textElem) {
    disableBlockingMsg();
    
    let splitMsg = msg.split('\n');
    let msgCon = createElement('div', 'blocking_message', ['fs-1', 'text-break', 'font-monospace']);
    let overlay = createElement('div', 'blocking_overlay', 'fadein');

    $('body').append(msgCon);
    $('body').append(overlay); 
    
    for(let i = 0; i < splitMsg.length; i++){
        let msgElem = createElement(textElem == null ? 'h1' : textElem);
        msgCon.append(msgElem);
        msgElem.text(splitMsg[i]);
    }
    
    let elemPer = msgCon.width() / $(document).width();

    msgCon.css('left', 50 - (elemPer*100/2) + '%')

    if(color != null)
        msgCon.css('color', color);

    if(blockNav){
        msgCon.css('z-index', 9999);
        overlay.css('z-index', 9999);
    }
    
    if(time != null)
        setTimeout(disableBlockingMsg, time);
}

//Remove the blocking message
function disableBlockingMsg() {
    $('#blocking_message').remove();
    $('#blocking_overlay').remove();
    //$('#blocking_overlay').css('z-index', '')    
}

//Gets the file name from the URL
function getPageName(){
    let path = window.location.pathname;
    var page = path.split("/").pop();
    return page;
}

function refreshPage() {
    window.location.reload();
    window.location.href = window.location.href;
}

function openInNewTab(url) {
 window.open(url, '_blank').focus();
}


//Is the element currently in view?
//param offset  : Offset from the top or bottom of the page, to take nav or footer in to account.
$.fn.isInViewport = function (offset) {
    var oset = {top:0, bottom:0, left:0, right:0}; 
    
    for(key in offset){
        oset[key] = offset[key];
    }
    
    let elementTop = $(this).offset().top + oset.bottom;
    let elementBottom = elementTop + $(this).outerHeight() + oset.top;

    let viewportTop = $(window).scrollTop() ;
    let viewportBottom = viewportTop + $(window).height() ;

    return elementBottom > viewportTop && elementTop < viewportBottom;
};


//Capitalize a letter at an index of a string.
function capitalIndex(string, index) {
  return string.charAt(index).toUpperCase() + string.slice(index + 1);
}

//Capitalize the first letter of a string
function capitalFirst(string){
    return capitalIndex(string, 0);
}

//timeout synchronously
function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

$.fn.cssImportant = function(key, value){
    $(this).css(key, '');
    let str = key + ':' + value + ' !important';
    $(this).attr('style', function(i,s) { return (s || '') + str })
}

//Load a JSON file from a specific URL
//*Only being used in devMode, untested for regular operation.
const loadData = async (url, fn) => {
	try {
		const response = await fetch(url);
		let data = await response.json();
        fn(data);
		console.log(data);
	} catch (err) {
			console.warn(err);
	}	
};