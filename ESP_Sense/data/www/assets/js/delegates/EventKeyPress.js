var EventKeyPress = new Set();

//Create the event to call the functions in the set.
$(document).keypress(CallEventKeyPress);

function CallEventKeyPress(e){  
    EventKeyPress.Invoke(e);  
}

