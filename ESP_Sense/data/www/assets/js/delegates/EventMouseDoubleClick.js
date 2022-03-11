var EventMouseDoubleClick = new Set();

//Create the event to call the functions in the set.
$(document).on('dblclick',CallEventMouseDoubleClick);

function CallEventMouseDoubleClick(e){  
    EventMouseDoubleClick.Invoke(e);    
}

