var EventWindowScroll = new Set();

//Create the event to call the functions in the set.
$(window).scroll(CallEventWindowScroll);

function CallEventWindowScroll(e){  
    EventWindowScroll.Invoke(e); 
}

