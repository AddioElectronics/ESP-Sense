var EventReady = new Set();

//Create the event to call the functions in the set.
$(document).ready(CallEventReady);

function CallEventReady(e){  
    EventReady.Invoke(e);    
}

