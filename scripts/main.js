
var v = 0, d = 0;

digitalWrite(2, 0);

setInterval(function() {
	if (0 == v) d = 1;
	else if (255 <= v) d = -1;
	v += d;
	analogWrite(2, v);
	console.log(v);
}, 10);

save(); // write the code above to flash
