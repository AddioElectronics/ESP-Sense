var EventMouseClick = new Set();

//Create the event to call the functions in the set.
$(document).on('click',CallEventMouseClick);

function CallEventMouseClick(e){  
    EventMouseClick.Invoke(e);    
}

