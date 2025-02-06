const char HTML_INDEX[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=480"/>
<style>
	* {
		border-radius:4px; transition:filter 0.5s, opacity 1s;
	}
	body {
		background:#222; color:#bbb; font-family:sans-serif; height:100vh; line-height:0;
	}
	div#topBar {
		margin-bottom:20px !important; text-align:center;
	}
	div:not(#form) {
		background:#0004; line-height:26px; margin:4px; font-size:10px; padding:2px; border:1px solid #000; overflow:hidden; max-height:500px;
	}
	div#form {
		width:400px; margin:0 auto; background:#fff1; padding:10px; border-radius:10px; box-shadow:0 0 32px 16px #0004; border:2px solid #000; margin-top:20px;
	}
	div#popup {
		background:#0006; backdrop-filter:blur(6px); line-height:50px; width:430px; position:fixed; top:50%; left:50%; transform:translateX(-50%) translateY(-50%); border:2px solid #fff4; font-size:18px; text-align:center; box-shadow:0px 10px 40px 0px #000; opacity:0;
	}
   	div#console {
		background:#0006; color:#0c8; backdrop-filter:blur(6px); font:13px monaco,monospace; overflow:hidden scroll; max-width:800px;  width:90%; height:70%; position:fixed; top:50%; left:50%; transform:translateX(-50%) translateY(-50%); border:2px solid #fff4; box-shadow:0px 10px 40px 0px #000; word-break:break-all; display:none;
   	}
	input, select {
		background:#0006; color:#fff; width:50%; height:20px; border:2px solid #0007; float:right;
	}
	input:disabled {
		opacity:0.3;
	}
	input[type=checkbox] {
		width:12px; height:12px; float:none; vertical-align:middle;
	}
	input[type="file"] {
		display:none;
	}
	button {
		color:#fff; width:100%; height:50px; margin-top:4px; border:2px solid #000; font-weight:bold; box-shadow:1px 1px 0 0 #fff4 inset, -1px -1px 4px 0 #0004 inset;
	}
	select {
		width:calc(50% + 7px) !important; height:26px !important;
	}
	select#wifiScan {
		display:none;
	}
	.title {
		color:#fff; font-weight:bold; margin:0 0 0 6px; cursor:pointer;
	}
	#btnScan {
	    display:none; font-size:20px; float:right; margin-right:6px; cursor:pointer;
	}
	.cursor {
		width:8px; height:14px; background:#888 !important; margin:0 !important; -webkit-animation-name:blink; -webkit-animation-duration:1s; -webkit-animation-iteration-count:infinite;
	}
	.spin {
		display:inline-block; margin-right:10px; border:2px solid #fff; border-radius:10px 10px 10px 0; width:10px; height:10px; animation:spin 1.0s linear infinite;
	}
	.popupNow {
		transition:opacity 0s;
	}
	#avatar {
		width:32px; height:32px; margin:0 !important; position:absolute; border-radius:8px; text-align:center; box-shadow: 0 0 8px 2px #fff6;
	}
	#popupBar {
		background:#ff83; height:50px; position:fixed; left:2px;
	}
	#btnSave {
		background:#333; margin-top:20px;
	}
	#btnConsole {
		background:#333;
	}
	#btnReboot {
		background:#333;
	}
	#btnUpdate {
		background:#733;
	}
	@keyframes spin {
		0% {transform:rotate(0deg);} 100% {transform:rotate(360deg);}
	}
	@keyframes blink {
		from {opacity:1.0} to {opacity:0.2};
	}
</style>
<script>
	_  = (q) => { return document.querySelector(q); }
	__ = (q) => { return document.querySelectorAll(q); }
	EventTarget.prototype.addEL = function(t, l, o) { return this.addEventListener(t, l, o); }

	document.addEL("DOMContentLoaded", (e) => {
		var spin='<span class="spin"></span>';
		var sysinfoTimer = 0;

		// AJAX request
		ajaxGet = (url, cb = () => {}) => {
			var xhr = new XMLHttpRequest();
			xhr.open('GET', url);
			xhr.onreadystatechange = () => {
				if (xhr.readyState === 4)
					if (xhr.status === 200) { cb(xhr.response); } else { popup('HTTP ERROR! '+xhr.statusText, 1000, 1, 250); }
			};
			xhr.send();
		}

		ajaxPost = (url, data, cb = () => {}) => {
			var xhr = new XMLHttpRequest();
			xhr.open('POST', url, true);
			xhr.setRequestHeader('Content-Type', 'application/json');
			xhr.onreadystatechange = () => {
				if (xhr.readyState === 4)
					if (xhr.status === 200) { cb(xhr.response); } else { popup('HTTP ERROR! ' + xhr.statusText, 1000, 1, 250); }
			};
			xhr.send(JSON.stringify(data));
		}

		updateAvatar = (s, z = 32) => {
			let h = 0, c = document.createElement('canvas'), x = c.getContext('2d');
			c.width = c.height = z;
			for (let i = 0; i < s.length; i++)
				h = ((h << 5) - h + s.charCodeAt(i)) & 0xFFFFFFFF;
			h = Math.abs(h);
			x.fillStyle = `hsl(${h%360},${50+h%50}%,${50+h%20}%)`;
			for (let i = 0; i < 15; i++) {
				if ((h >> i) & 1) {
					let col = i % 3, row = i / 3 | 0;
					x.fillRect(col * z/5, row * z/5, z/5, z/5);
					x.fillRect((4-col) * z/5, row * z/5, z/5, z/5);
				}
			}
			_("#avatar>img").src = c.toDataURL();
		};

		// Pop-up box
		popup = (txt, offDelay=0, bar=0, barDelay=0) => {
			var el = _('#popup');
			if (txt !== undefined) {
				_('#form').style.opacity = 0.4;
				el.innerHTML = '<span id="popupBar" style="transition:width ' + barDelay + 'ms linear; width:0%"></span>';
				el.classList.add('popupNow'); el.style.opacity = 1; el.style.display = ""; el.innerHTML += txt;
				if (offDelay > 0) setTimeout(popup, offDelay);
				setTimeout(() => { _('#popupBar').style.width = (bar-1).toString()+'%'; }, 50);
			} else {
				_('#form').style.opacity = 1;
				el.classList.remove('popupNow'); el.style.opacity = 0; setTimeout(() => { el.style.display = "none" }, 1000);
			}
		}

		// Mouse hover on buttons-event
		__('button').forEach((elX) => {
			elX.addEL('mouseover', () => { elX.style.filter = "brightness(1.3)"; }, false);
			elX.addEL('mouseout',  () => { elX.style.filter = ""; }, false);
		});

		// Disable elements when checkboxes change
		updateCheckbox = (el) => {
			var chk = el.value = el.checked;
			if (el.id == "wwwAuth") {
				__('input:not([name="wwwAuth"]):not([readonly])[name^="www"]')
				.forEach((elX) => { elX.disabled = chk == true ? false : true});
			}
			if (el.id == "netDHCP") {
				__('input:not([id="netDHCP"]):not([readonly])[id^="net"]')
				.forEach((elX) => { elX.disabled = chk });
			}
			if (el.id == "mqttHass") {
				__('input[id="mqttTopicHass"]')
				.forEach((elX) => { elX.disabled = chk == true ? false : true});
			}
			if (el.id == "wifiManual") {
				_('input[id="wifiSSID"]').style.display = chk == true ? "block" : "none";
				_('select[id="wifiScan"]').style.display = chk == true ? "none" : "block";
				_('span[id="btnScan"').style.display = chk == true ? "none" : "block";
			}
		}

		// Change on any checkbox, trigger updateCheckbox()
		__('input[type="checkbox"]').forEach((elX) => {
			elX.addEL('change', () => { updateCheckbox(elX); }, false);
		});

		// Change on #wifiScan select, set #wifiSSID input
		_('select#wifiScan').addEL('change', (el) => { _('input[id="wifiSSID"]').value = el.target.value; }, false);

		// Load configuration
		loadConfig = () => {
			popup(spin + 'LOADING CONFIGURATION');
			ajaxGet('/load', (resp) => {
				var cfg = JSON.parse(resp);

				if (cfg['safemode'] == 1)
					_('body').style.background = "repeating-linear-gradient(45deg,#222,#222 30px,#522 30px,#522 60px)";

				_("#version").innerHTML = " Version " + cfg['version'];
				updateAvatar(cfg['deviceName']);

				Object.keys(cfg).forEach((key) => {
					__('input').forEach((elX) => {
						if (elX.id == key) {
							elX.value = cfg[key];
							if (elX.type == "checkbox") {
								elX.checked = cfg[key] == 'true' || cfg[key] == true ? true : false;
								elX.dispatchEvent(new Event('change'));
							}
						}
					});

					__('select').forEach((elX) => {
						if (elX.id == key) {
							// Find matching option from select element
							for (i=0; i < elX.options.length; i++) { if (elX.options[i].value == cfg[key]) elX.selectedIndex = i; }
						}
					});
				});
				popup();
			});
		}

		// Load sysinfo
		loadSysinfo = () => {
			let i=0;
			ajaxGet('/sysinfo', (resp) => {
				var sys = JSON.parse(resp);

				__("#topBar>div").forEach( (e) => { e.remove(); })
				Object.keys(sys).forEach((key) => { _("#topBar").insertAdjacentHTML('beforeEnd', "<div>" + key + "<input id='sysinfo" + i++ + "' disabled readonly value='" + sys[key] + "'></div>" ); })
			});
		}

		// Load wifiscan
		loadWifiscan = () => {
			ajaxGet('/wifiscan', (resp) => {
				var scan = JSON.parse(resp);
				var ssidFound = false;
				var elInput = _('input[id="wifiSSID"]');
				var elManual = _('input[id="wifiManual"]');
				var elSelect = _("#wifiScan");

				// Clear list, add status
				__("#wifiScan>option").forEach( (e) => { e.remove(); });
				elSelect.insertAdjacentHTML('beforeEnd', "<option selected disabled>"+ scan.status +"</option>" );

				if ("ap" in scan) {
					elSelect.insertAdjacentHTML('beforeEnd', "<option disabled>Only unique SSIDs listed</option>");

					// Unique filter, populate SSIDs to select list
					scan.ap = [...new Map(scan['ap'].map(item => [item['ssid'], item])).values()];
					Object.keys(scan.ap).forEach((key,i) => {
						elSelect.insertAdjacentHTML('beforeEnd', "<option value='" + scan.ap[key].ssid + "'>"+ scan.ap[key].ssid +"</option>");
						if (scan.ap[key].ssid == elInput.value) { elSelect.selectedIndex = i+2; ssidFound = true; }
					});
					// Enable manual checkbox
					if (ssidFound) { elManual.checked = false; updateCheckbox(elManual); }
				} else {
					// No "ap" array found, rescan..
					setTimeout(loadWifiscan, 2000);
				}
			});
		}

		// IP-address pattern
		__('input:not([id="netDHCP"])[id^="net"]').forEach((elX) => {
			elX.maxLength = 15;
			elX.pattern = '^(?!0)(?!.*\\.$)((1?\\d?\\d|25[0-5]|2[0-4]\\d)(\\.|$)){4}$';
		});

		// Generate a POST-request and submit config
		_('#formConfig').addEL("submit", function(e) {
			e.preventDefault();
			var data = {};
			var inputs = this.querySelectorAll('input, select');
			
			inputs.forEach(function(input) {
				if (input.tagName === 'SELECT') {
					data[input.id] = input.selectedOptions[0].value;
				} else {
					data[input.id] = input.value;
				}
			});

			popup(spin + 'SAVING CONFIGURATION');
			ajaxPost("/save", data, () => { loadConfig(); });
			return false;
		});

		// Reboot
		_('#btnReboot').addEL("click", (e) => {
			popup(spin + 'REBOOTING, PLEASE WAIT..', 1000*5, 100, 1000*5);
			ajaxGet("/reboot", () => { setTimeout(() => { location.reload(); }, 1000*5); });
		});

		// Click on hidden input-element
		_('#formUpdate').addEL("submit", function(e) {
			e.preventDefault();
			_('#inputUpdate').click();
		});

		// Create custom file-form and AJAX post it
		_('#inputUpdate').addEL("change", function(e) {
			popup(spin + "UPLOADING..", 0, 0, 100);

			var formdata = new FormData();
			formdata.append("file1", this.files[0]);

			var xhr = new XMLHttpRequest();
			xhr.upload.addEL("progress", function(e) {_('#popup span').style.width = (e.loaded/e.total*100).toString() + '%' }, false);
			xhr.addEL("load", function(e) {
				var txt = e.target.responseText;
				if (txt.toLowerCase().includes("reboot")) { _('#btnReboot').click(); } else { popup(txt, 2000); }
			}, false);
			xhr.addEL("error", function() { popup("UPLOAD FAILED!", 2000); }, false);
			xhr.addEL("abort", function() { popup("UPLOAD ABORTED!", 2000); }, false);
			xhr.open("POST", "/update");
			xhr.send(formdata);
		});

		// Open console
		_('#btnConsole').addEL("click", (e) => {
			var el = _("#console");
			el.style.display = "block";
			_('#form').style.opacity = 0.4;
		});

		// Close console
		_("body").addEL("click", (e) => {
			var el = _("#console");
			if (el.style.display != "none" && e.target.id != "btnConsole" && !el.contains(e.target)) {
				e.stopPropagation();
				e.preventDefault();

				el.style.display = "none";
				el.innerHTML = el.innerHTML.substring(el.innerHTML.lastIndexOf("<div"));
				_('#form').style.opacity = 1;
			}
		}, true);

		// Write to console
		toConsole = (txt) => {
			var el = _('#console');
			_(".cursor").insertAdjacentHTML('beforeBegin', txt+"<br>" );
			el.scrollTop = el.scrollHeight;
		}

		// Listen to events sent by server
		if (!!window.EventSource) {
			var evt = new EventSource('/events');
			evt.addEL('message', function(e) { toConsole('<font color="#fff">MSG:</font> '+e.data) }, false);
			evt.addEL('log', function(e) { toConsole('<font color="#fff">LOG:</font> '+e.data); }, false);
			evt.addEL('error', function(e) {
				if (e.target.readyState != EventSource.OPEN) { toConsole('<font color="#f44">Disconnected!</font>'); }
			}, false);
		}

		// Create collapsible DIVs
		__('span:not([id="btnScan"])').forEach(function(elX) {
			var height = "", arrow = "&#9662";
			if (elX.innerText != "Device" && elX.innerText != "Network") { height = "24px"; arrow = "&#9656;"; }
			elX.parentElement.style["max-height"] = height;
			elX.innerHTML = arrow + " " + elX.innerHTML;

			elX.addEL("click", function(e) {
				var height = "", arrow = "&#9662";
				if (e.target.parentElement.style["max-height"] != "24px") { height = "24px"; arrow = "&#9656;"; }

				e.target.parentElement.style["max-height"] = height;
				e.target.innerHTML = arrow + e.target.innerHTML.slice(1)

				if (e.target.id == "version" && height == "") { sysinfoTimer = setInterval(loadSysinfo, 1000); } else { clearInterval(sysinfoTimer); }
			});
		});

		// Listen to change-events on deviceName input field, update avatar
		_('input[id=deviceName]').addEL("change", (e) => {
			updateAvatar(e.target.value);
		})

		// Start WIFI scan
		_('#btnScan').addEL("click", (e) => {
			loadWifiscan();
		});

		_("body").click();
		loadConfig();
		loadSysinfo();
		loadWifiscan();
	});
</script>
</head>
<body>
	<div id="form">
		<div id="avatar"><img src=""></div>
		<div id="topBar">UberMayhem for PortaPack
			<span id="version"> Version N/A</span>
		</div>
		<form id="formConfig" action="/save" method="post">
			<div style="background:#555">
				<span class="title">Device</span>
				<div>Device Name<input maxlength="16" pattern="^(([a-zA-Z]|[a-zA-Z][a-zA-Z0-9\-_]*[a-zA-Z0-9]))" id="deviceName"></div>
				<div>Device Description<input maxlength="32" id="deviceDesc"></div>
			</div>
			<div style="background:#453">
				<span class="title">Network</span>
				<div>WIFI SSID ( Manual: <input id="wifiManual" type="checkbox" checked="true">)
					<input maxlength="32" id="wifiSSID">
					<select id="wifiScan"></select>
					<span id="btnScan">&#8635;</span>
				</div>
				<div>WIFI Password<input maxlength="32" id="wifiPass" type="password"></div>
				<div>
					<span class="title">IP Settings</span>
					<div>IP Address ( Dynamic: <input id="netDHCP" type="checkbox">)<input id="netIP"></div>
					<div>Netmask<input id="netMask"></div>
					<div>Gateway<input id="netGW"></div>
					<div>DNS<input id="netDNS"></div>
					<div>MAC Address<input id="netMAC" disabled readonly></div>
				</div>
			</div>
			<div style="background:#533">
				<span class="title">Web</span>
				<div>Web Login ( Enabled: <input id="wwwAuth" type="checkbox" checked="true">)<input maxlength="32" id="wwwUser"></div>
				<div>Web Password<input maxlength="32" id="wwwPass" type="password"></div>
			</div>
			<div style="background:#534">
				<span class="title">MQTT</span>
				<div>MQTT Server<input maxlength="64" id="mqttServer"></div>
				<div>MQTT Port<input maxlength="6" pattern="[0-9]+" id="mqttPort"></div>
				<div>MQTT User<input maxlength="16" id="mqttUser"></div>
				<div>MQTT Password<input maxlength="16" id="mqttPass" type="password"></div>
				<div>
					<span class="title">Topic Settings</span>
					<div>Topic Root<input maxlength="16" id="mqttTopicRoot"></div>
					<div>Topic Sub<input maxlength="16" id="mqttTopicSub"></div>
					<div>Topic Pub<input maxlength="16" id="mqttTopicPub"></div>
				</div>
				<div>
					<span class="title">Home-Assistant Integration</span>
					<div>HA Autodiscover ( Enabled: <input id="mqttHass" type="checkbox" checked="true">)<input maxlength="16" id="mqttTopicHass"></div>
					<div>HA Color Mode<select id="colorMode"><option value="rgb">RGB</option><option value="hs">HS</option><option value="xy">XY</option></select></div>
				</div>
			</div>
			<div style="background:#345">
				<span class="title">FastLED</span>
				<div>
					<span class="title">Default Values</span>
					<div>Brightness<input maxlength="3" pattern="[0-9]+" id="ledBrightness"></div>
					<div>Color (HEX: RRGGBB)<input minlength="6" maxlength="6" pattern="[0-9a-fA-F]+" id="ledColor" style="text-transform:uppercase;"></div>
				</div>
				<div>
					<span class="title">Advanced Settings</span>
					<div>Number Of LEDs<input maxlength="3" pattern="[0-9]+" id="ledCount" disabled readonly></div>
					<div>GPIO Pin<input maxlength="2" pattern="[0-9]+" id="ledPin" disabled readonly></div>
				</div>
			</div>
			<button id="btnSave" type="submit">SAVE</button>
		</form>
		<button id="btnConsole" type="submit">CONSOLE</button>
		<button id="btnReboot" type="button">REBOOT</button>
		<form id="formUpdate" action="/update" enctype="multipart/form-data" method="post">
			<button id="btnUpdate" type="submit">FIRMWARE UPDATE</button>
			<input id="inputUpdate" type="file" accept=".bin" id="file1" id="file1">
		</form>
	</div>
	<div id="popup"></div>
	<div id="console"><div class="cursor"></div></div>
</body>
</html>
)=====";