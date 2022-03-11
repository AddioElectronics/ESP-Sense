var EventWindowResize = new Set();

//Create the event to call the functions in the set.
$(window).resize(CallEventWindowResize);

function CallEventWindowResize(e){  
    EventWindowResize.Invoke(e); 
}

