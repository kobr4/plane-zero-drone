var rpio = require('rpio');

var options = {
        gpiomem: false,          /* Use /dev/gpiomem */
        mapping: 'physical',    /* Use the P1-P40 numbering scheme */
}

rpio.init(options);

var pin = 33;           /* P33 GPIO */
var pin2 = 35;           /* P35 GPIO */
var range = 1000;       /* LEDs can quickly hit max brightness, so only use */
var clockdiv = 256;    /* Clock divider (PWM refresh rate), 8 == 2.4MHz */

/*
 * Enable PWM on the chosen pin and set the clock and range.
 */
rpio.open(pin, rpio.PWM);

rpio.open(pin2, rpio.PWM);

rpio.pwmSetClockDivider(clockdiv);
rpio.pwmSetRange(pin, range);
rpio.pwmSetRange(pin2, range);

var center = 60;
var p1 = center;
var p2 = center;

function centering() {
  console.log("Centering");
  rpio.pwmSetData(pin,p1);
  rpio.pwmSetData(pin2,p2);
//  p1 = p1 + 2;
//  p2 = p2 - 2;
}

function close() {
 console.log("Exiting");
 rpio.pwmSetData(pin,0);
 rpio.pwmSetData(pin2,0);
 clearInterval(15);
 rpio.open(pin, rpio.INPUT);
 rpio.open(pin2, rpio.INPUT);
 process.exit()
}

setInterval(centering,500)
 
setTimeout(close, 6000);
