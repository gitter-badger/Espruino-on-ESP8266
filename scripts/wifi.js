wifi.scan(function(aps) {
	for (var n = 0; n < aps.length; n++) {
		console.log(aps[n]);
	}
});
