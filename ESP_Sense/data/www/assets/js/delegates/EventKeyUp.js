var EventKeyUp = new Set();

//Create the event to call the functions in the set.
$(document).keyup(CallEventKeyUp);

function CallEventKeyUp(e){  
    EventKeyUp.Invoke(e);  
}

