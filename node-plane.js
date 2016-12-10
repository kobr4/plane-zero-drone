var net = require('net');
var rpio = require('rpio');

var options = {
        gpiomem: false,          /* Use /dev/gpiomem */
        mapping: 'physical',    /* Use the P1-P40 numbering scheme */
}

rpio.init(options);
var s1 = 33;
var s2 = 35;
var range = 1000;       /* LEDs can quickly hit max brightness, so only use */
var clockdiv = 256;    /* Clock divider (PWM refresh rate), 8 == 2.4MHz */


rpio.open(s1, rpio.PWM);
rpio.open(s2, rpio.PWM);

rpio.pwmSetClockDivider(clockdiv);
rpio.pwmSetRange(s1, range);
rpio.pwmSetRange(s2, range);

var s1pw = 60;
var s2pw = 60;

rpio.pwmSetData(s1,s1pw);
rpio.pwmSetData(s2,s2pw);

var server = net.createServer(function(socket) {
	socket.on('data', function(data) {
	console.log('Received data: ' + data);	   
	   var command = String(data).split(':')
	   switch(command[0]) {
	     case "s1" :  
                 s1pw =  parseInt(command[1]);
                 rpio.pwmSetData(s1,s1pw);
	         console.log("sending to s1 : "+command[1]);
		 break;
	     case "s2" : 
                 s2pw = parseInt(command[1]);
                 rpio.pwmSetData(s2,s2pw);
		 console.log("sending to s2 : "+command[1]);
                 
		 break;
	     case "m1" :  
                rpio.pwmSetData(m1, parseInt(command[1]));
                console.log("sending to m1 : "+command[1]);
		break;
	   }
	   return
	 })
});

server.listen(1234, '192.168.0.18');
console.log('-- Start listening at port 1234 --')
