Pebble.addEventListener("showConfiguration",
	function (e) {
		console.log("Configuration requested");
		Pebble.openURL("http://perezdecastro.org/pebble/beats-config.html");
	}
);

var TOGGLE_FLAGS = {
	batterybar: 1 << 0,
	hhmmdisplay: 1 << 1,
};

Pebble.addEventListener("webviewclosed",
	function (e) {
		var json = JSON.parse(decodeURIComponent(e.response));
		console.log("Configuration window returned: " + JSON.stringify(json));
		var toggles = 0;
		for (var key of TOGGLE_FLAGS) {
			if (typeof json[key] == undefined || !!json[key]) {
				toggles = toggles | TOGGLE_FLAGS[key];
			}
		}
		Pebble.sendAppMessage({
			"PERSIST_KEY_FONT": "" + json.font,
			"PERSIST_KEY_TOGGLES": toggles,
		}, function (e) {
				console.log("Settings sent successfully");
			},
			function (e) {
				console.log("Failed to send settings: " + JSON.stringify(e));
			}
		);
	}
);
