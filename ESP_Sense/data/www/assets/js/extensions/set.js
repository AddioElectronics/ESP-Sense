Set.prototype.AddRange = function(range){
    
    Debug.Log("Set.prototype.AddRange()", Set.debugindex, 0, true);
    
     if(range != null)
                if(Array.isArray(range)){
                    for(const fn of range){
                        if(fn != null){
                            this.add(fn);
                        }else{
                             Debug.Warn("Set.prototype.AddRange() :: Cannot add, value is null.", Set.debugindex, 0, "force");
                        }
                    }
                }else{
                    this.add(range);
                }
}

//Just like Set.add but includes a debug function
Set.prototype.Add = function(add){
    
    if(add != null){
        Debug.Log("Set.prototype.Add()", "set", 0);    
        this.add(add);
    }else{
        Debug.Warn("Set.prototype.Add() :: Cannot add, value is null.", Set.debugindex, 0, "force");
    }
}

Set.prototype.Invoke = function(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o , p, q, r, s, t, u, v, w, x, y, z){
    
  //Only continue if there is atleast 1 function in the array.
    if(this.size > 0){
        //Call each function in the set.
        for(const fn of this) {
            fn(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o , p, q, r, s, t, u, v, w, x, y, z);
        }
    }
}

Set.prototype.Merge = function(b){
    
    Debug.Log("Set.Merge()", Set.debugindex, 1);
    
    if(this == null && b == null)
        return null;
    
    var set = new Set();   
    
    if(this != null){
        if(this.size > 0){
            for(const fn of this) {
            set.add(fn);
            }
        }
    }
    
    if(b != null){
        if(b.size > 0){
            for(const fn of b) {
            set.add(fn);
            }
        }
    }

    return set;
}
 
Set.debugindex = ["set", "extension"];

class SetExt extends Set{
    
    static Merge(a, b){
        
        Debug.Log("SetExt.Merge() :: {a.size="+ (a != null ? a.size : "null") + ", b.size="+ (b != null ? b.size : "null") +"}", Set.debugindex, 1);
        
        if(a == null && b == null)
            return null;
        
        var set = new Set();   
        
        if(a != null){
            if(a.size > 0){
                for(const fn of a) {
                set.add(fn);
                }
            }
        }
        
        if(b != null){
            if(b.size > 0){
                for(const fn of b) {
                set.add(fn);
                }
            }
        }
    
        return set;
    }
    
    static AddRange(set, functions){
        if(functions != null)
                if(Array.isArray(functions)){
                    for(const fn of functions){
                         set.add(fn);
                    }
                }else{
                    set.add(functions);
                }
        
        return set;
    }
    
}