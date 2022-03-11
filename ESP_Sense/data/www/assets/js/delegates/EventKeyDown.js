var EventKeyDown = new Set();

//Create the event to call the functions in the set.
$(document).keydown(CallEventKeyDown);

function CallEventKeyDown(e){  
    EventKeyDown.Invoke(e);  
}

