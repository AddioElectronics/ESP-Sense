var events = {};

function invokeEvent(name){
    events[name].Invoke();
}

function onEvent(name, fn){
    events[name].add(fn);
}

function removeFromEvent(name, fn){
    events[name].delete(fn);
}

function createEvent(name){
    events[name] = new Set();
}

function deleteEvent(name){
    try{
    if(events[name] != undefined)
        delete events[name];
    }catch{}
}

createEvent('auth');
createEvent('reset');
createEvent('resetting');
createEvent('status');




