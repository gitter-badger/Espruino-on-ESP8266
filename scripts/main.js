digitalWrite(15, 0);
digitalWrite(12, 0);
digitalWrite(13, 0);
digitalWrite(2, 0);

setInterval(function() {
	var v = 100 - analogRead();
	v = (v - 20.0) * 1.2;
	analogWrite(2, v);
}, 10);

save();
