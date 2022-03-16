/*
A class to help customize how messages are logged to the console.
Use the parameters to only show messages when they meet the requirements set by the user.

@param Debugging    : When false none of the functions will
@param Index        : Functions with non matching Index will not be completed, unless the index is -1
@param Levels       : Functions with level matching anything in this array will be displayed, even if out of range from low and high.
@param LowLevel     : Functions with level lower than LowLevel will not be displayed.
@param HighLevel    : Functions with level higher than Highlevel will also not be displayed.

::NOTE::    When calling these functions, just remember they will not display the line that originally called the function in the console.
            Its a good idea to include either the name of the class or file, as well as the function name, or else it may be very hard to find.
*/
class Debug{
    
    /*
    @message        : The message you want to be displayed. If this is the only argument passed, then as long as Debugging is true it will be logged to the console. 
    @index          : The message will only be displayed if debugging is on, and Debug.Index matches the index, can be a int/string or what ever.
    @level          : The message will only be displayed if debugging is on, and the level is with in the Low and High level debugging range, or if the index is a match. 
                      A good habbit to follow is the more rare (or important) of a message, the higher the level.
    @displayLine    : Do you want to display the line of code calling the Log? (true, false, "short") 
                  
    ::level:: You do not need to follow these, but it gives you a good idea for how I envisioned the levels to be. Feel free to fill them in yourself.
        0   :   Called a ton of times on page load, and frequently during runtime.
        1   :   Called multiple times on page load, and often during runtime.
        2   :   Called once or twice on page load, and sometimes during runtime.
        3   :   
        4   :
        5   :   User Inputs, from things frequent things such as button clicks.
        6   :   User Inputs, less frequent things such as saving/applying/canceling forms.
        7   :
        8   :
        9   :
        10  :   Rarely ever called.
    */  
    static Log(message, index, level, displayLine, objects){
        if(!Debug.Debugging)
            return;
        
        var hasLogged = false;
        
        switch(Debug.AddDisplayLine(displayLine)){
            case true:
                message += ErrorExt.Line();
                break;
            case "short":
                message += ErrorExt.LineShort();
                break;
        }

        if(arguments.length > 1)
            if(typeof(index) == typeof(""))
                index = index.toLowerCase();
        
        if(Debug.Logging || Debug.Logging == 1){
        switch(arguments.length){
            case 1:
                console.log(message);
                hasLogged = true;
                break;
            case 2:
                if(ArrayExt.MatchAny(index, Debug.Index)){
                    console.log(message);
                    hasLogged = true;
                }
                break;
            case 3:
            case 4:
            case 5:
                if(((level <= Debug.HighLevel && level >= Debug.LowLevel) || ArrayExt.MatchAny(level, Debug.Levels)) && (ArrayExt.MatchAny(index, Debug.Index) || Debug.Index == -1)){
                    console.log(message);
                    hasLogged = true;
                }
                break;
            
        }
        }else if(Debug.Logging == "always" || Debug.Logging == 2){
            console.log(message);
            hasLogged = true;
        }
        
        
        if(arguments.length > 4)
            if(hasLogged)
                if(Debug.Objects == "always" || Debug.Objects == true){
                    if(Array.isArray(objects)){
                        objects.forEach(function(value){
                            console.log(value);
                        })
                    }else{
                        console.log(objects);
                    }
                }
    }
    
    static Warn(message, index, level, displayLine){
        if(!Debug.Debugging)
            return;
        
        switch(Debug.AddDisplayLine(displayLine)){
            case true:
                message += ErrorExt.Line();
                break;
            case "short":
                message += ErrorExt.LineShort();
                break;
        }

        if(arguments.length > 1)
            if(typeof(index) == typeof(""))
                index = index.toLowerCase();
        
        if(Debug.Warning || Debug.Warning == 1){
            switch(arguments.length){
                case 1:
                    console.warn(message);
                    break;
                case 2:
                    if(ArrayExt.MatchAny(index, Debug.Index))
                        console.warn(message);
                    break;
                case 3:
                case 4:
                    if(((level <= Debug.HighLevel && level >= Debug.LowLevel) || ArrayExt.MatchAny(level, Debug.Levels)) && (ArrayExt.MatchAny(index, Debug.Index) || Debug.Index == -1))
                        console.warn(message);
                    break;
            }
        }else if(Debug.Warning == "always" || Debug.Warning == 2){
            console.warn(message);
        }
    }
    
    static WarnAlert(message, index, level, displayLine){
        if(!Debug.Debugging)
            return;
        
        Debug.Alert(message, index, level);
        
        switch(Debug.AddDisplayLine(displayLine)){
            case true:
                message += ErrorExt.Line();
                break;
            case "short":
                message += ErrorExt.LineShort();
                break;
        }
        
        if(arguments.length > 1)
            if(typeof(index) == typeof(""))
                index = index.toLowerCase();
        
        if(Debug.Warning || Debug.Warning == 1){
            switch(arguments.length){
                case 1:
                    console.warn(message);
                    break;
                case 2:
                    if(ArrayExt.MatchAny(index, Debug.Index)){
                        console.warn(message);
                    }
                    break;
                case 3:
                case 4:
                    if(((level <= Debug.HighLevel && level >= Debug.LowLevel) || ArrayExt.MatchAny(level, Debug.Levels)) && (ArrayExt.MatchAny(index, Debug.Index) || Debug.Index == -1)){
                        console.warn(message);
                    }
                    break;
            }
        }else if(Debug.Warning == "always" || Debug.Warning == 2){
            console.warn(message);
        }
    }
    
    static Error(message, index, level, displayLine){
        if(!Debug.Debugging)
            return;
        
        switch(Debug.AddDisplayLine(displayLine)){
            case true:
                message += ErrorExt.Line();
                break;
            case "short":
                message += ErrorExt.LineShort();
                break;
        }
            
        
        if(arguments.length > 1)
            if(typeof(index) == typeof(""))
                index = index.toLowerCase();
        
        if(Debug.Errors || Debug.Errors == 1){
            switch(arguments.length){
                case 1:
                    throw new Error(message);
                case 2:
                    if(ArrayExt.MatchAny(index, Debug.Index))
                        throw new Error(message);
                    break;
                case 3:
                case 4:
                    if(((level <= Debug.HighLevel && level >= Debug.LowLevel) || ArrayExt.MatchAny(level, Debug.Levels)) && (ArrayExt.MatchAny(index, Debug.Index) || Debug.Index == -1))
                        throw new Error(message);
                    break;
            }
        }else if(Debug.Errors == "always" || Debug.Errors == 2){
            throw new Error(message);
        }
    }
    
    static ErrorAlert(message, index, level, displayLine){
        if(!Debug.Debugging)
            return;
        
        Debug.Alert(message, index, level);
        
        switch(Debug.AddDisplayLine(displayLine)){
            case true:
                message += ErrorExt.Line();
                break;
            case "short":
                message += ErrorExt.LineShort();
                break;
        }
        
        if(arguments.length > 1)
            if(typeof(index) == typeof(""))
                index = index.toLowerCase();
        
        if(Debug.Errors || Debug.Errors == 1){
            switch(arguments.length){
                case 1:
                    throw new Error(message);
                case 2:
                    if(ArrayExt.MatchAny(index, Debug.Index)){
                        throw new Error(message);
                    }
                    break;
                case 3:
                case 4:
                    if(((level <= Debug.HighLevel && level >= Debug.LowLevel) || ArrayExt.MatchAny(level, Debug.Levels)) && (ArrayExt.MatchAny(index, Debug.Index) || Debug.Index == -1)){
                        throw new Error(message);
                    }
                    break;
            }
        }else if (Debug.Errors == "always" || Debug.Errors == 2){
            throw new Error(message);
        }
    }
    
    static Alert(message, index, level){
        if(!Debug.Debugging)
            return;
        
        if(arguments.length > 1)
            if(typeof(index)== typeof(""))
                index = index.toLowerCase();
        
        if(Debug.Alerting || Debug.Alerting == 1){
            switch(arguments.length){
                case 1:
                    alert(message);
                    break;
                case 2:
                    if(ArrayExt.MatchAny(index, Debug.Index))
                        alert(message);
                    break;
                case 3:
                case 4:
                    if(((level <= Debug.HighLevel && level >= Debug.LowLevel) || ArrayExt.MatchAny(level, Debug.Levels)) && (ArrayExt.MatchAny(index, Debug.Index) || Debug.Index == -1))
                        alert(message);
                    break;
            }
        }else if(Debug.Alerting == "always" || Debug.Alerting == 2){
            alert(message);
        }
    }
    
    //Call this to display a confirmation message.
    //Meant for testing delegates.
    static Test(){           
        console.log("Test function has been called." + ErrorExt.Line());
    }
    
    static PrintArray(array, displayLine, message){
        
        if(message == null)
            message = "";
        
         switch(Debug.AddDisplayLine(displayLine)){
            case true:
                message += ErrorExt.Line();
                break;
            case "short":
                message += ErrorExt.LineShort();
                break;
        }
        
        console.log("::Start Printing Array:: " + message);
        for(var i = 0; i < array.length; i++){
            console.log(array[i]);
        }
        console.log("::Finished Printing Array::");
    }
    
    static PrintKeys(obj, displayLine, message){
        
        if(message == null)
            message = "";
        
        var array = Object.keys(obj);
        
         switch(Debug.AddDisplayLine(displayLine)){
            case true:
                message += ErrorExt.Line();
                break;
            case "short":
                message += ErrorExt.LineShort();
                break;
        }
        
        console.log("::Start Printing Keys:: " + message);
        for(var i = 0; i < array.length; i++){
            console.log(array[i]);
        }
        console.log("::Finished Printing Keys::");
    }
    
    
    static UpdateSettings(index, levels, low, high, line){
        Debug.ChangeIndex(index);
        Debug.Changelevels(levels);
        Debug.ChangeLowLevel(low);
        Debug.ChangeHighLevel(high);
        Debug.ChangeDisplayLine(line);
    }
    
    static ChangeIndex(index){
        
        if(Array.isArray(index)){
            
            var a = [];
            
            for(const c in index){
                if(typeof(c) == typeof(""))
                    a.push(c.toLowerCase());
                else
                    a.push(c);
            }
            
            
        }else{
             if(typeof(c) == typeof(""))
                 index = index.toLowerCase();
            
            Debug.Index = index;
        }
        
    }
    
    static Changelevels(levels){
        if(!Array.isArray(levels)){
            Debug.Levels = [levels];
        }else{
            Debug.Levels = levels;
        }
    }
    
    static ChangeLowLevel(level){
        if(typeof(level) === typeof(0))
            Debug.LowLevel = level;
        else
            console.log("Debug.ChangeLowLevel() :: Cannot change Debug.LowLevel, level("+level+") is not a Number!");
    }
    
    static ChangeHighLevel(level){
        if(typeof(level) === typeof(0))
            Debug.HighLevel = level;
         else
             console.log("Debug.ChangeHighLevel() :: Cannot change Debug.HighLevel, level("+level+") is not a Number!");
    }
    
    static ChangeDisplayLine(state){
        
        if(typeof(state) == typeof(""))
            state = state.toLowerCase();
        
        Debug.DisplayLine = state;
    }
    
    
    static AddDisplayLine(dl){        
        if((Debug.DisplayLine && dl !== false) || dl || (typeof dl == typeof("") ? dl.toLowerCase() : null) == "force" || Debug.DisplayLine == "force"){
            return true;
        }else if((Debug.DisplayLine == "short"  && dl != false) || (typeof dl == typeof("") ? dl.toLowerCase() : null) == "short" || Debug.DisplayLine == "forceshort"){
            return "short";
        }
        
        return false;
    }
    
}

//Will not send any messages to the console if false. (Debug.Log messages only)
Debug.Debugging = true; 

//User can set what they want passed to the console.
//(true || 1), (false || null || 0), ("always" || 2)
Debug.Logging = true;
Debug.Objects = true;
Debug.Warning = true;
Debug.Alerting = true;
Debug.Errors = true;
//Will only send messages with matching or no index, unless this is -1.
Debug.Index = ["dock", "toolbar", "elementdockpoint", "pagedockpoint"];    //, "block", "menu", "block-menu", "block-edit-menu"
//Will only send mesages with matching levels.
Debug.Levels = [];
//Will only send messages with matching, greater, or no level
Debug.LowLevel = 0;   
//Will only send messages with matching, less than, or no level
Debug.HighLevel = 10;   
//Do you want the calling Script/Line to be displayed?
//If false, you will only see the message, with the debug.js line.
//If "short", the script path will be excluded.
//If "force", it will show for every function (unlcess false in Debug.Log).
Debug.DisplayLine = true;



//Debug.DL = Object.freeze({false:0, true:1, short:2, force: 3, forceshort:4});

//We don't want to have to worry about capitals, so lets just make them all lower case.
if(!Array.isArray(Debug.Index))
    if(typeof Debug.Index === typeof "")
    Debug.Index = Debug.Index.toLowerCase();
else{
    for(var i = 0; i < Debug.Index.length; i++){
        if(typeof Debug.Index[i] === typeof "")
            Debug.Index[i] = Debug.Index[i].toLowerCase();
    }
}


if(typeof(Debug.DisplayLine) == typeof(""))
    Debug.Displayline = Debug.DisplayLine.toLowerCase();


console.log("Debug :: {Debugging=" + Debug.Debugging + " | Logging="+Debug.Logging+ " | Warnings="+Debug.Warning+ " | Alerts="+Debug.Alerting+ " | Errors="+Debug.Errors+ " :: Index=[" + Debug.Index + "] | LowLevel=" + Debug.LowLevel + " | HighLevel=" + Debug.HighLevel + " | Levels=["+Debug.Levels.toString()+"]}");