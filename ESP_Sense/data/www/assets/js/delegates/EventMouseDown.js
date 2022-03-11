var EventMouseDown = new Set();

var isMouseDown = false;

//Create the event to call the functions in the set.
$(document).mousedown(CallEventMouseDown);

function CallEventMouseDown(e){      
    isMouseDown = true;
    EventMouseDown.Invoke(e); 
}

