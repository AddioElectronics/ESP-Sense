//EventMouseUp.js must be included after EventMouseDown
var EventMouseUp = new Set();

//Create the event to call the functions in the set.
$(document).mouseup(CallEventMouseUp);

function CallEventMouseUp(e){      
    isMouseDown = false;
    EventMouseUp.Invoke(e); 
}

