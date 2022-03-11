var EventMouseMove = new Set();

//Create the event to call the functions in the set.
$(document).mousemove(CallEventMouseMove);

function CallEventMouseMove(e){  
    EventMouseMove.Invoke(e); 
}

//Add the TrackMouse function to the array.
EventMouseMove.add(TrackMouse);

var mouseX = 0;
var mouseY = 0;

function TrackMouse(e){
     mouseX = e.pageX;
     mouseY = e.pageY;    
}