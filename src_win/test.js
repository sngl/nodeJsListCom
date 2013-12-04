

function callback(results)
{
    
    console.log(arguments);
    
    
    
}


var lister = require('./build/Release/listcom');


lister.list(callback);