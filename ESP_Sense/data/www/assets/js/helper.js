var blockingMessage = `<h1 id="blocking_message" class="fadein">Heading</h1>`;
var blockingOverlay = `<div id="blocking_overlay" class="fadein"></div>`;

function Show(obj) {
    $(obj).show();
    $(obj).removeClass('d-none');
}

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

function removeAttr(obj, attr){    
    if($.isArray(obj)){
        obj.forEach(elem => $(elem).removeAttr(attr));
    }else{
        $(obj).removeAttr(attr)
    }
}

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

function enableBlockingMsg(msg, blockNav, color, time, textElem) {
    disableBlockingMsg();
    
    let splitMsg = msg.split('\n');
    
    let msgCon = createElement('div', 'blocking_message', ['fs-1', 'text-break', 'font-monospace']);

    $('body').append(msgCon);
    $('body').append(blockingOverlay); 
    
    for(let i = 0; i < splitMsg.length; i++){
        let msgElem = createElement(textElem == null ? 'h1' : textElem);
        msgCon.append(msgElem);
        msgElem.text(splitMsg[i]);
    }
    
    let elemPer = msgCon.width() / $(document).width();

    msgCon.css('left', 50 - (elemPer*100/2) + '%')

    if(color != null)
        msgCon.css('color', color);

    if(blockNav)
        msgCon.css('z-index', 9999);
    
    if(time != null)
        setTimeout(disableBlockingMsg, time);
}

function disableBlockingMsg() {
    $('#blocking_message').remove();
    $('#blocking_overlay').remove();
    //$('#blocking_overlay').css('z-index', '')    
}

function getPageName(){
    let path = window.location.pathname;
    var page = path.split("/").pop();
    return page;
}

function refreshPage() {
    window.location.reload();
    window.location.href = window.location.href;
}

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



function capitalIndex(string, index) {
  return string.charAt(index).toUpperCase() + string.slice(index + 1);
}

function capitalFirst(string){
    return capitalIndex(string, 0);
}

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

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