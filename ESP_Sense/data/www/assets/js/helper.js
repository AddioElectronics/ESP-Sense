var blockingMessage = `<h1 id="blocking_message" class="fadein">Heading</h1>`;
var blockingOverlay = `<div id="blocking_overlay" class="fadein"></div>`;

function Show(obj) {
    $(obj).show();
    $(obj).removeClass('d-none');
}

function disableObj(obj) {
    $(obj).attr('disabled','');
}

function enableObj(obj) {
    $(obj).removeAttr('disabled', null);
}


function enableBlockingMsg(msg) {
    
    $('body').append(blockingMessage);
    $('body').append(blockingOverlay);

    $('#blocking_message').text(msg);
}

function disableBlockingMsg() {
    $('#blocking_message').remove();
    $('#blocking_overlay').remove();
}

function refreshPage() {
    window.location.reload();
    window.location.href = window.location.href;
}

$.fn.isInViewport = function () {
    let elementTop = $(this).offset().top;
    let elementBottom = elementTop + $(this).outerHeight();

    let viewportTop = $(window).scrollTop();
    let viewportBottom = viewportTop + $(window).height();

    return elementBottom > viewportTop && elementTop < viewportBottom;
};


function addAttr(obj, attr){
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


function createElement(type, id, classes, attrs, text, isHtml, eClick){
    var elem = document.createElement(type);
    elem = $(elem);
    
    if(id != null)
    elem.attr('id', id);
    
    if(classes != null){
        if(Array.isArray(classes)){
            for(var i = 0; i < classes.length; i++){
                elem.addClass(classes[i]);
            }
        }else{
            elem.addClass(classes);
        }
    }
    
    if(attrs != null){
        if(Array.isArray(attrs)){
            for(var i = 0; i < attrs.length; i++){
                addAttr(elem, attrs[i]);
            }
        }else{
            addAttr(elem, attrs);
        }
    }
    
    if(text != null && text.length > 0){
        if(isHtml == undefined || isHtml == false){
            elem.text(text);
        }else{
            elem.html(text);
        }
    }
    
    return elem;
    
}

function capitalIndex(string, index) {
  return string.charAt(index).toUpperCase() + string.slice(index + 1);
}

function capitalFirst(string){
    return capitalIndex(string, 0);
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