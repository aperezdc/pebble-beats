Pebble.addEventListener("showConfiguration",
	function (e) {
		console.log("Configuration requested");
		Pebble.openURL("http://perezdecastro.org/pebble/beats-config.html");
	}
);

Pebble.addEventListener("webviewclosed",
	function (e) {
		var json = JSON.parse(decodeURIComponent(e.response));
		console.log("Configuration window returned: " + JSON.stringify(json));
		Pebble.sendAppMessage({ "PERSIST_KEY_FONT": "" + json.font },
			function (e) {
				console.log("Settings sent successfully");
			},
			function (e) {
				console.log("Failed to send settings: " + JSON.stringify(e));
			}
		);
	}
);
