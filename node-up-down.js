var rpio = require('rpio');

var options = {
        gpiomem: false,          /* Use PWM (requires sudo= */
        mapping: 'physical',    /* Use the P1-P40 numbering scheme */
}

rpio.init(options);

var pin = 35;           /* Used GPIO pin */
var range = 1000;       /* Range of PWM */
var max = 100;          /*   the bottom 8th of a larger scale */
var clockdiv = 256;       /* Clock divider (PWM refresh rate), 8 == 2.4MHz */

/*
 * Enable PWM on the chosen pin and set the clock and range.
 */
rpio.open(pin, rpio.PWM);
rpio.pwmSetClockDivider(clockdiv);
rpio.pwmSetRange(pin, range);

var MAX = 60;
var MIN = 110;

var power = MAX;

function powerInput() {
  rpio.pwmSetData(pin,power);
}

function close() {
 console.log("EXITING");
 rpio.pwmSetData(pin,0);
 clearInterval(15);
 rpio.open(pin, rpio.INPUT);
 process.exit()
}

setInterval(powerInput,1000);

var stdin = process.openStdin();
stdin.on('data', function(chunk) { console.log("Got chunk: " + chunk); 
  if (power == MAX) {  
    console.log("LOW");
    power = MIN;
  }
  else close();
});

