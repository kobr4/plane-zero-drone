var net = require('net');
var servoblaster = require('servoblaster');

var stream = servoblaster.createWriteStream();

stream.write({pin : 5,value: 90});
stream.write({pin : 6,value: 90});
stream.write({pin : 7,value: 100});

function startVideo(remoteIp,port) {
var exec = require('child_process').exec;
var cmd = 'raspivid -t 0 -w 1280 -h 720 -hf -ih -fps 20 -o - | nc -u '+remoteIp+' '+port;

exec(cmd, function(error, stdout, stderr) {
  console.log("Running command: "+cmd);
  console.log(stdout);
});
}

var server = net.createServer(function(socket) {
	socket.on('data', function(data) {
	console.log('Received data: ' + data);	   
	   var command = String(data).split(':')
	   switch(command[0]) {
	     case "s1" :  
                 var s1pw =  parseInt(command[1]);
                 stream.write({pin : 5,value: s1pw});
	         console.log("sending to s1 : "+command[1]);
		 break;
	     case "s2" : 
                 var s2pw = parseInt(command[1]);
                 stream.write({pin : 6,value: s2pw});
		 console.log("sending to s2 : "+command[1]);      
		 break;
	     case "s1s2":
                var s1pw =  parseInt(command[1]);
                 stream.write({pin : 5,value: s1pw});
                 //console.log("sending to s1 : "+command[1]);
                 var s2pw = parseInt(command[2]);
                 stream.write({pin : 6,value: s2pw});
                 //console.log("sending to s2 : "+command[1]);
                 break;
	     case "m1" : 
                var m1pw = parseInt(command[1]); 
                stream.write({pin : 7,value: m1pw});
                console.log("sending to m1 : "+command[1]);
		break;
	     case "video" :
	        var remoteIp = socket.remoteAddress;
		var port = parseInt(command[1]);	
  		console.log("starting video to "+remoteIp+":"+port);
		startVideo(remoteIp,port);
	   }
	   return
	 })
});

process.on( 'SIGINT', function() {
  console.log( "\nGracefully shutting down from SIGINT (Ctrl-C)" );
  console.log("Exiting");
  stream.end();
  process.exit();
})

server.listen(1234, '192.168.1.1');
console.log('-- Start listening at port 1234 --')
