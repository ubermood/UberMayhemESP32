const char HTML_PORTAPACK[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=540"/>
<style>
	* { transition: opacity 0.5s; border-radius: 4px; }
	*:focus { outline: none; }
	:root {
		--angleZ: 0deg; --progress: 0%; --progressOpacity: calc(120% + -100 * clamp(0%, var(--progress), 1%));
	}
	@keyframes flash {
		50% { box-shadow: 0 0 16px 4px #f888; }
	}
	body {
		background: #222;
		color: #bbb;
		font-size: 11px;
		height: 100vh;
		margin:0;
		font-family: monospace;
	}
	input, select {
		background: #0006;
		color: #ddd;
		border: 0;
		padding: 0;
		line-height: 20px;
		width: 130px;
		height: 24px;
	}
	input[type='button'] {
		background: #822;
		color: #fff;
	}
	#divBanner {
		background: #f008;
		position: fixed;
		display: none;
		top: 0;
		width: 100%;
		color: #fff;
		line-height: 15px;
		border-bottom: 2px solid #000;
		padding: 10px;
		backdrop-filter: blur(4px);
		cursor: pointer;
		z-index: 1;
	}
	#divBannerClose {
		width: 20px;
		height: 20px;
		border: 2px solid #fff6;
		right: 40px;
		position: fixed;
		text-align: center;
		line-height: 20px;
		font-weight: bold;
		border-radius: 4px;
	}
	#divTopBar {
		background: #0004;
		font-family: sans-serif;
		margin-bottom: 20px !important;
		text-align: center;
		line-height: 26px;
	}
	#divMain {
		width: 450px;
		margin: 0 auto;
		background: #fff1;
		padding: 10px;
		border-radius: 10px;
		box-shadow: 0 0 32px 16px #0004;
		border: 2px solid #000;
		margin-top: 20px;
	}
	#wsInput {
		width: 410px;
		opacity: var(--progressOpacity);
	}
	#wsOutput {
		background: #0008;
		color: #ddd;
		font-size: 7pt;
		height: 250px;
		border: 1px solid #000;
		width: -webkit-fill-available;
	}
	#divStatus {
		color: #0008;
		font-size: 32px;
		width: 32px;
		height: 32px;
		line-height: 32px;
		position: absolute;
		border-radius: 8px;
		text-align: center;
		border: 3px solid #000;
		box-shadow: 0 0 8px 2px #fff6;
	}
	#divCompass {
		background: #044;
		width: 100px;
		height: 100px;
		border: 3px dashed #fff2;
		border-radius: 50%;
		transform: rotate(var(--angleZ));
	}
	#divCompass div { position: absolute; font-weight: bold; font-size: 12pt; }
	#divCompass div:nth-child(1) { left: 45px; top: 10px; }
	#divCompass div:nth-child(2) { right: 80px; top: 42px; }
	#divCompass div:nth-child(3) { left: 45px; bottom: 10px; }
	#divCompass div:nth-child(4) { left: 80px; top: 42px; }
	#divCompass div:nth-child(5) { left: calc(50% - 8px); top: calc(50% - 8px); background: #fff4; width: 16px; height: 16px; border-radius: 50%; }
	#divTilt {
		position: relative;
		background: #fff8;
		width: 10px;
		height: 10px;
		border: 1px solid #000;
		border-radius: 50%;
		top: -49px;
		left: 47px;
		margin-top: -10px;
	}
	#divScreenProgress {
		background: linear-gradient(to right, #fffa 0%, #fffa var(--progress), #0000 var(--progress));
		width: 200px;
		height: 8px;
		position: absolute;
		z-index: 1;
		margin: 18px;
		border-radius: 4px;
		border: 1px solid #000;
		opacity: calc(var(--progress) * 100);
		box-shadow: 0 0 0 1px #fff;
	}
	#cvsScreen {
		border: 1px solid #000;
		background: #0006;
		border-radius: 4px;
		filter: brightness(1.25);
		opacity: calc(var(--progressOpacity)* 3);
		cursor: crosshair;
	}
	#divScreenCtrl {
		font-size: 20pt;
		color: #fff;
		margin-left: 6px;
	}
	#divScreenCtrl > div {
		background: #484;
		width: 50px;
		height: 50px;
		line-height: 50px;
		float: left;
		margin: 3px;
		border-radius: 8px;
		text-align: center;
		cursor: pointer;
		opacity: var(--progressOpacity);
	}
	#divSensors { background: #446; }
	#divScreen { background: #454; position: relative; }
	#divConsole { background: #552; }
	#divFBrowser { background: #523; }
	#divPayloads { background: #733; }
	#divConsole > .divSubBlock,  #divFBrowser > .divSubBlock {
		flex-direction: column;
	}
	#divPayloads > .divSubBlock {
		flex-wrap: wrap;
	}
	#divFiles {
		overflow: scroll;
		font-size: 12pt;
		background: #0004;
		height: 290px;
		border: 1px solid #000;
		width: -webkit-fill-available;
		opacity: var(--progressOpacity);
	}
	#divFActions {
		display: flex;
		justify-content: flex-end;
		font-size: 25px;
		cursor: pointer;
		opacity: 0.25;
	}
	#divFActions > div {
		width: 30px;
		text-align: center;
		border: 1px solid;
		border-radius: 5px;
		margin: 4px;
	}
	#divFActions > div:hover, #divScreenCtrl > div:hover, .fItem:hover:not(:has(.fItem:hover)), input[type="button"]:hover { filter: brightness(1.5); }
	#divFActions > #btnYES {
		margin-right: -36px;
		display: relative;
		transition: margin-right 0.25s;
	}
	#divPayloads .divValue {
		background: linear-gradient(0deg, #400 0px, #0000 60px);
		height: 60px;
		display: flex;
		flex-direction: column;
		justify-content: space-between;
		opacity: var(--progressOpacity);
	}
	.title {
		color: #fff;
		font-weight: bold;
		margin: 0 0 0 6px;
		line-height: 26px;
		font-family: sans-serif;
		cursor: pointer;
	}
	.divBlock {
		border: 1px solid #000;
		margin-top: 10px;
		border-radius: 4px;
		overflow:hidden;
		margin: 4px;
		padding: 2px;
	}
	.divSubBlock {
		border: 1px solid #000;
		background: #0004;
		display: flex;
		align-items: center;
		padding: 4px;
		margin: 2px;
		border-radius: 4px;
		overflow: hidden;
	}
	.divValue {
		background: #fff2;
		margin: 1px;
		border: 1px solid #000;
		border-radius: 4px;
		padding: 3px;
		float: left;
		width: min-content;
	}
	.fItem { display: flex; flex-direction: row; padding: 0 3px; margin-left: 10px; cursor: pointer; }
	.fTypeD { color: #882; }
	.fTypeF { color: #288; }
	.fSelect { color: #fff; margin-left: 5px; transition: margin 0.1s; }
	.divFPanel {
		display: flex;
		justify-content: space-between;
		width: -webkit-fill-available;
	}
	.manInput { background: #700; }
</style>
<script>
	var ws;
	var ssl = location.protocol === 'https:' ? true : false;
	var wsState = false;
	var shDataInput = "";
	var shDataState = false;
	var shDataArr = [];
	var shBytesRead = 0;
	var shBytesToRead = 0;
	var PROMPT = 'ch> ';
	var manualInput = [];

	var shDataInputDebug = "";

	var currentGpsData = {
		"latitude": 0.0,
		"longitude": 0.0,
		"altitude": 0.0,
		"speed": 0.0,
		"day": 1,
		"month": 1,
		"year": 1,
		"hour": 0,
		"minute": 0,
		"second": 0
	};
	var currentEnvData = {
		"temperature": 0.0,
		"humidity": 0.0,
		"pressure": 0.0
	};
	var currentOriData = {
		"angle": 0.0,
		"tilt": 0.0,
		"roll": 0.0,
		"pitch": 0.0
	};
	var currentLightData = {
		"light": 0
	}

	// Minimized helper functions
	_  = (q) => { return document.querySelector(q); }
	__ = (q) => { return document.querySelectorAll(q); }
	EventTarget.prototype.addEL = function(t, l, o) { return this.addEventListener(t, l, o); }

	// Round decimals
	roundDecimals = (n, d = 8) => { const res = Math.round(parseFloat(n) * Math.pow(10, d)) / Math.pow(10, d); return res ? res : 0; }

	// Convert quaternion to heading in degrees
	calcQuaternion = (q) => {
		let [x, y, z, w] = q;
		const yaw   = Math.atan2(2 * (w * z + x * y), 1 - 2 * (y * y + z * z));
		const pitch = Math.asin(2 * (w * y - z * x));
		const roll  = Math.atan2(2 * (w * x + y * z), 1 - 2 * (x * x + y * y));

		const headAngle = (360 - yaw * (180 / Math.PI)) % 360;
		const pitchAngle = pitch * (180 / Math.PI);
		const rollAngle = roll * (180 / Math.PI);
		const tiltAngle = Math.sqrt(pitch * pitch + roll * roll) * (180 / Math.PI);

		//const tiltAngle2 = pitch >= 0 ? pitchAngle : -pitchAngle;
		//return [headAngle, tiltAngle2, rollAngle, pitchAngle];
		const tiltAngle2 = tiltAngle >= 0 ? tiltAngle : -tiltAngle;
		return [headAngle, tiltAngle2, pitchAngle, rollAngle];
	}

	// ------------------------------------------------------

	document.addEL("DOMContentLoaded", (e) => {
		// ------------------------------------------------------
		// Read sensors

		// Gyroscope
		if ("Gyroscope" in window) {
			const s = new Gyroscope({ frequency: 25 });
			s.addEL("reading", (e) => { _("#divGyrVal").value = roundDecimals(s.x,4) +" | "+ roundDecimals(s.y,4) +" | "+ roundDecimals(s.z,4); });
			s.start();
		}

		// Accelerometer
		if ("Accelerometer" in window) {
			const s = new Accelerometer({ frequency: 25 });
			s.addEL("reading", (e) => { _("#divAclVal").value = roundDecimals(s.x,4) +" | "+ roundDecimals(s.y,4) +" | "+ roundDecimals(s.z,4);	});
			s.start();
		}

		// Orientation
		if ("AbsoluteOrientationSensor" in window) {
			const s = new AbsoluteOrientationSensor({ frequency: 25 });
			s.addEL("reading", (e) => {
				let [head, tilt, roll, pitch] = calcQuaternion(s.quaternion);

				currentOriData = {
					"angle": roundDecimals(head, 4),
					"tilt": roundDecimals(tilt, 4),
					"roll": roundDecimals(roll, 4),
					"pitch": roundDecimals(pitch, 4)
				};

				if ( !manualInput.includes("divOriAng") ) _("#divOriAng").value = currentOriData.angle;
				if ( !manualInput.includes("divOriTil") ) _("#divOriTil").value = currentOriData.tilt;

				_(":root").style.setProperty('--angleZ', currentOriData.angle + 'deg');
				_("#divTilt").style.top = -49 + currentOriData.pitch / 2 + 'px';
				_("#divTilt").style.left = 47 + currentOriData.roll / 2 + 'px';
			});
			s.start();
		} else {
			_("#divCompass").style.background="#622";
			_("#divCompass").style.color="#844";
		}

		// Light
		if ("AmbientLightSensor" in window) {
			const s = new AmbientLightSensor();
			s.addEL("reading", (e) => {
				currentLightData.light = parseInt(s.illuminance);

				if ( !manualInput.includes("divLuxVal") ) _("#divLuxVal").value = currentLightData.light;
			});
			s.start();
		} else {
			_("#divBannerText").innerHTML = 'Sensor(s) missing!<br>You may need to enable sensor extra classes.<br><br>Link: chrome://flags/#enable-generic-sensor-extra-classes';
			_("#divBanner").style.display = "block";
		}

		// Check Secure Context
		if (!window.isSecureContext) {
			_("#divBannerText").innerHTML = 'Page is not secure!<br>You may need to add "http://' + location.hostname + '" as a trusted URL.<br><br>Link: chrome://flags/#unsafely-treat-insecure-origin-as-secure'
			_("#divBanner").style.display = "block";
		}

		// ------------------------------------------------------
		// Functions

		// GPS
		getGps = () => {
			navigator.geolocation.getCurrentPosition( (e) => {
				let d = new Date();

				currentGpsData = {
					"latitude": roundDecimals(e.coords.latitude, 8),
					"longitude": roundDecimals(e.coords.longitude, 8),
					"altitude": roundDecimals(e.coords.altitude, 4),
					"speed": roundDecimals(e.coords.speed, 2),
					"day": d.getDate(),
					"month": d.getMonth() + 1,
					"year": d.getFullYear(),
					"hour": d.getHours(),
					"minute": d.getMinutes(),
					"second": d.getSeconds(),
					//"thousand": d.getSeconds()
				};

				if ( !manualInput.includes("divGpsLat") ) _("#divGpsLat").value = currentGpsData.latitude;
				if ( !manualInput.includes("divGpsLon") ) _("#divGpsLon").value = currentGpsData.longitude;
				if ( !manualInput.includes("divGpsSpd") ) _("#divGpsSpd").value = currentGpsData.speed;
				if ( !manualInput.includes("divGpsAlt") ) _("#divGpsAlt").value = currentGpsData.altitude;
			});
		}
		setInterval(getGps, 1000);

		// Log to console
		wsLog = (tag, msg) => {
			var el = _('#wsOutput');
			el.value += (tag ? tag + ' ' : '') + msg + '\n';

			if (el.scrollTop >= el.scrollHeight - 350) // Autoscroll if at bottom
				el.scrollTop = el.scrollHeight;
		}

		// Screen renderer
		screenRender = () => {
			return new Promise((resolve) => {
				sendMessage('ppshell=screenframeshort', (e) => {
					// Render screen
					const ctx = _("#cvsScreen").getContext("2d");
					ctx.fillRect(0, 0, 240, 320);

					e.data.forEach((row, y) => {
						if (row.length < 230)
							return;

						for (let x = 0; x < 240 && x < row.length; x++) {
							let b = row.charCodeAt(x) - 32;
							ctx.fillStyle = `rgb(${(b>>4&3)<<6},${(b>>2&3)<<6},${(b&3)<<6})`;
							ctx.fillRect(x, y + 1, 1, 1);
						}
					});
					resolve();
				});
			});
		}

		// File browser
		fileBrowse = (file) => {
			return new Promise((resolve) => {
				if (file.indexOf(" ") != -1) { resolve(); return; }

				let div = _("div.fTypeD[data-file='" + file + "']");
				div = div ? div : _("#divFiles"); // Root folder or subfolder?

				if (div.childElementCount > 0) {
					if (file != "/")
						while (div.childElementCount) { div.children.item(0).remove(); } // Collapse folder
				} else {
					// List files
					sendMessage('ppshell=ls ' + file, (e) => {
						if (!e) { resolve(); return; }

						fArray = e.data.sort((a, b) => b.endsWith('/') - a.endsWith('/') || a.localeCompare(b)); // Sort folders and files
						for (let i = 0; i < fArray.length; i++) {
							let fType = fArray[i].slice(-1) == "/" ? "fTypeD" : "fTypeF"; // Check filetype
							let pFile = '/' + (file + '/' + fArray[i]).replace(/^\/+|\/+$/g, ''); // Path incl. filename

							div.insertAdjacentHTML('beforeEnd', '<div class="fItem"><div class="' + fType + '" data-file="' + pFile + '">' + fArray[i] + '</div></div>');
						}
						resolve();
					});
				}
			});
		}

		// WS Connection handler
		wsConnect = () => {
			// Create a WebSocket connection
			ws = new WebSocket( (ssl ? 'wss' : 'ws') + '://' + window.location.host + '/ws');

			if (ws) {
				// When WS establishes connection
				ws.addEL('open', () => {
					wsState = true;
					_("#divStatus").style.background = "#0a4";
					_("#divStatus").innerHTML = "&#10003;";

					wsLog('**', 'Connection opened');

					// Initial sync
					sendMessage("getPayloads");
					sendMessage('getPayloadState');
					setTimeout(async () => {
						await fileBrowse("/");
						await screenRender();
					}, 3000);
				});

				// When WS connection fails
				ws.addEL('error', () => {
					wsState = false;
					ws.close();
				});

				// When WS closes connection
				ws.addEL('close', () => {
					wsState = false;
					shDataState = false
					wsLog('**', 'Connection closed');

					_("#divStatus").style.background = "#a04";
					_("#divStatus").innerHTML = "&#10007;";
					_(":root").style.setProperty('--progress', '0%');
					setTimeout(() => { wsConnect(); }, 3000);
				});

				// When receiving WS messages
				ws.addEL('message', (e) => {
					if (e.data.length == 1)
						shDataState = true;

					// Shell messages handler
					if (shDataState) {
						shDataInput += e.data;
						shBytesRead += e.data.length - 2 * (e.data.split("\r\n").length - 1); // Subtract linefeeds

						const lines = shDataInput.split("\r\n");
						shDataArr.push(...lines.slice(0, -1));
						shDataInput = lines.pop();

						if (shDataArr.length > 1) {
							const cmd = shDataArr[0];
							const isScreen = cmd.startsWith("screenframeshort");
							const isFile = cmd.startsWith("fread");

							if (isScreen || isFile) {
								if (isScreen) {
									shBytesToRead = 240 * 320;
									_("#divScreen > .divSubBlock").appendChild(_("#divScreenProgress"));
								} else
									_(".divFPanel").appendChild(_("#divScreenProgress"));

						 		let percent = Math.floor((100 / shBytesToRead) * (shBytesRead - cmd.length));
								_(":root").style.setProperty('--progress', percent >= 100 ? 0 : percent + '%');
							} else
								lines.forEach(line => wsLog('', line));
						}

						// Check for promfBrowse
						if (shDataInput.endsWith(PROMPT)) {
							if (shDataInput.length > PROMPT.length)
								shDataArr.push(shDataInput.slice(0, -PROMPT.length));

							shDataInput = "";
							shBytesRead = 0;
							shDataState = false;
						}
					}

					// Other messages handler
					else {
						wsLog('', e.data);

						let json = {};
						try { json = JSON.parse(e.data); } catch (e) { return; }

						// Payloads status
						if (json["payloads"] !== undefined)
							addPayloads(json.payloads);
						if (json["payloadstate"] !== undefined)
							updPayloads(json.payloadstate);
					}
				});
			}
		}

		sendMessage = (msg, cb) => new Promise((resolve, reject) => {
			if (!wsState) {
				wsLog('!!', 'Cannot send, not connected');
				return reject();
			};
			if (!msg || shDataState) {
				wsLog("!!", "Another shell process still running");
				return reject();
			};

			ws.send(msg);
			wsLog('=>', msg);

			// Shell message
			shDataState = msg.toLowerCase().startsWith("ppshell=") ? true : false;

			const check = () => {
				if (shDataState)
					return setTimeout(check, 100);

				const result = { cmd: shDataArr[0], data: shDataArr.slice(1) };
				shDataArr = [];
				(cb || resolve)(result);
			};
			check();
		});

		// Unselect file
		fileUnselect = () => {
			if (_(".fSelect"))
				_(".fSelect").classList.remove("fSelect");

			_("#divFActions").style.opacity = "";
			_("#btnYES").style["margin-right"] = "";
		}

		// Add payload blocks
		addPayloads = (payloads) => {
			const target = _("#divPayloads > .divSubBlock");
			target.innerHTML = "";

			Object.entries(payloads).forEach(([key, value]) => {
				let html, name;
				const type = key.slice(0, 3);

				switch (type) {
					case 'btn':
					case 'txt':
						name = value;
						html = type == 'txt' ? '<input type="text" placeholder="Parameter">' : '';
						break;
					case 'sel':
						name = Object.keys(value)[0];
						const options = value[name].map((opt, i) => '<option value="' + i + '">' + opt + '</option>').join('');
						html = `<select>${options}</select>`;
						break;
				}

				html += '<input type="button" value="&#9021;" data-payload="' + name + '">';
				target.insertAdjacentHTML('beforeEnd', '<div class="divValue">' + name + html +'</div>');
			});
		}

		updPayloads = (payload) => {
			let css = payload.state == "started" ? "flash 0.5s infinite" : "";

			// Remove animation from all and add it on running paylod div
			__('#divPayloads .divValue[style*="animation"]').forEach(e => e.style.animation = "");
			_('#divPayloads .divValue:has(input[data-payload="' + payload.name + '"])').style.animation = css;
		}

		// ------------------------------------------------------
		// Event listeners

		// Close banner
		_("#divBannerClose").addEL('click', () => {
			_("#divBanner").style.display = "none";
		});

		// Enter key in wsInput field
		_("#wsInput").addEL('keypress', (e) => {
			if (e.key === 'Enter')
				sendMessage(_('#wsInput').value);
		});

		// Screen control
		_("#btnY").addEL('click', () => { screenRender(); });
		_("#btnX").addEL('click', () => { sendMessage('ppshell=touch 0 0'); });
		_("#btnR").addEL('click', () => { sendMessage('ppshell=button 1'); });
		_("#btnL").addEL('click', () => { sendMessage('ppshell=button 2'); });
		_("#btnD").addEL('click', () => { sendMessage('ppshell=button 3'); });
		_("#btnU").addEL('click', () => { sendMessage('ppshell=button 4'); });
		_("#btnC").addEL('click', () => { sendMessage('ppshell=button 5'); });
		_("#btnCCW").addEL('click', () => { sendMessage('ppshell=button 7'); });
		_("#btnCW").addEL('click', () => { sendMessage('ppshell=button 8'); });

		// Mouse click on screen
		_("#cvsScreen").addEL('mousedown', (e) => {
			const r = e.target.getBoundingClientRect();
			const x = parseInt(e.clientX - r.left);
			const y = parseInt(e.clientY - r.top);

			sendMessage('ppshell=touch '+ x +' '+ y, (e) => { screenRender(); });
		});

		// File select
		_("#divFiles").addEL('click', (e) => {
			fileUnselect();

			if (e.target.className == "fTypeD")
				fileBrowse(e.target.dataset["file"]);
			if (e.target.className == "fTypeF") {
				// Select file and reveal file actions
				e.target.classList.add("fSelect");
				_("#divFActions").style.opacity = 1;

				// Get file size
				_("#divFInfo").innerHTML = "";
				sendMessage('ppshell=filesize '+ e.target.dataset["file"], (e) => {
					_("#divFInfo").innerHTML = "Size: " + e.data[0] + " bytes";
				});
			}
		});

		// File download
		_("#btnDL").addEL('click', () => {
			if (_(".fSelect")) {
				const filename = _(".fSelect").dataset["file"];

				(async () => {
					await sendMessage('ppshell=fclose');
					await sendMessage('ppshell=fopen ' + filename);
					await sendMessage('ppshell=fseek 0');

					sendMessage('ppshell=filesize '+ filename, (e) => {
						let size = parseInt(e.data[0]);
						shBytesToRead = size * 2; // 2 bytes per actual byte in size

						sendMessage('ppshell=fread '+ size, (e) => {
							// Read file fom PP
							const data = e.data.slice(0, -1);
							const bytes = data.flatMap(str => str.match(/.{1,2}/g).map(byte => parseInt(byte, 16)));
							if (bytes.length !== size) {
								_(":root").style.setProperty('--progress', '0%');
								wsLog("!!", "Error downloading file!");
								return;
							}

							// Download file
							const arrUint = new Uint8Array(bytes);
							const blob = new Blob([arrUint], { type: 'application/octet-stream' });
							const url = URL.createObjectURL(blob);
							const a = document.createElement('a');

							a.href = url;
							a.download = filename.split('/').pop();
							a.click();
							URL.revokeObjectURL(url);
						});
					});
				})();
			}
		});

		// File remove
		_("#btnRM").addEL('click', () => {
			if (_(".fSelect"))
				_("#btnYES").style["margin-right"] = "0px";
		});

		// File remove - confirm
		_("#btnYES").addEL('click', () => {
			if (_(".fSelect"))
				sendMessage('ppshell=unlink ' + _(".fSelect").dataset["file"], () => {
					_(".fSelect").remove();
					fileUnselect();
				});
		});

		// Run payloads
		_("#divPayloads .divSubBlock").addEL("click", (e) => {
			if (e.target.type == "button") {
				if (e.target.parentNode.querySelector("input[type='text']"))
					sendMessage('setPayload={"name": "' + e.target.dataset["payload"] + '"}');
					// "param1":"' + e.target.parentNode.querySelector("input").value + '"}');
				else if (e.target.parentNode.querySelector("select"))
					sendMessage('setPayload={"name": "' + e.target.dataset["payload"] + '"}');
					// "param1":"' + e.target.parentNode.querySelector("select").value + '"}');
				else
					sendMessage('setPayload={"name":"' + e.target.dataset["payload"] + '"}');

				sendMessage('togglePayload');
			}
		})

		// Collapsible blocks
		__('span').forEach(function(elX) {
			var height = "", arrow = "&#9662";

			if (elX.innerText != "Screen" && elX.innerText != "Console")
				height = "24px"; arrow = "&#9656;";

			elX.parentElement.style["max-height"] = height;
			elX.innerHTML = arrow + " " + elX.innerHTML;

			elX.addEL('click', function(e) {
				var height = "", arrow = "&#9662";

				if (e.target.parentElement.style["max-height"] != "24px")
					height = "24px"; arrow = "&#9656;";

				e.target.parentElement.style["max-height"] = height;
				e.target.innerHTML = arrow + e.target.innerHTML.slice(1)
			});
		});

		_('#divSensors').addEL('click', (e) => {
			if (e.target.type == "text") {
				// Set textfield to manual input and add element id to array
				e.target.classList.add("manInput");

				if ( !manualInput.includes(e.target.id) )
					manualInput.push(e.target.id);
			}
		});

		// ------------------------------------------------------
		// Main

		wsConnect();

		// Send sensor data every 5s
		sendCurrentData = () => {
			if (wsState && !shDataState) {
				// Check manual input mode
				if (manualInput.includes("divGpsLat")) currentGpsData.latitude = roundDecimals(_("#divGpsLat").value);
				if (manualInput.includes("divGpsLon")) currentGpsData.longitude = roundDecimals(_("#divGpsLon").value);
				if (manualInput.includes("divGpsSpd")) currentGpsData.speed = roundDecimals(_("#divGpsSpd").value);
				if (manualInput.includes("divGpsAlt")) currentGpsData.altitude = roundDecimals(_("#divGpsAlt").value);
				if (manualInput.includes("divOriAng")) currentOriData.angle = roundDecimals(_("#divOriAng").value);
				if (manualInput.includes("divOriTil")) currentOriData.tilt = roundDecimals(_("#divOriTil").value);
				if (manualInput.includes("divEnvTem")) currentEnvData.temperature = roundDecimals(_("#divEnvTem").value);
				if (manualInput.includes("divEnvHum")) currentEnvData.humidity = roundDecimals(_("#divEnvHum").value);
				if (manualInput.includes("divEnvPrs")) currentEnvData.pressure = roundDecimals(_("#divEnvPrs").value);
				if (manualInput.includes("divLuxVal")) currentLightData.light = roundDecimals(_("#divLuxVal").value);

				sendMessage('setGps=' + JSON.stringify(currentGpsData));
				sendMessage('setOri=' + JSON.stringify(currentOriData));
				sendMessage('setEnv=' + JSON.stringify(currentEnvData));
				sendMessage('setLight=' + JSON.stringify(currentLightData));
			}
		}
		setInterval(sendCurrentData, 5000);

		// Check for incoming shell data timeout
		const checkDataTimeout = (() => {
			let lastBytes = 0, startTime = 0, lastActivity = 0;
			const TIMEOUT = 5000;
			return () => {
				const now = Date.now(), bytes = shBytesRead;
				if (shDataState) {
					if (!startTime)
						startTime = lastActivity = now;
					if (bytes > lastBytes)
						lastActivity = now;
					else if ((bytes === lastBytes && bytes > 0 && now - lastActivity > TIMEOUT) || (bytes === 0 && now - startTime > TIMEOUT)) {
						shDataState = false;
						_(":root").style.setProperty('--progress', '0%');
						wsLog("!!", `Timeout waiting for shell. ${bytes ? 'Transfer halted.' : 'No data received.'}`);
						startTime = lastActivity = 0;
					}
				} else
					startTime = lastActivity = 0;
				lastBytes = bytes;
			};
		})();
		setInterval(checkDataTimeout, 1000);
	});
</script>
</head>
<body>
	<div id="divBanner"><div id="divBannerClose">X</div><div id="divBannerText"></div></div>
	<div id="divMain">
		<div id="divStatus">-</div>
		<div id="divTopBar" class="divBlock">UberMayhem for PortaPack - Command and Control Center</div>
		<div id="divSensors" class="divBlock">
			<span class="title">Sensors</span>
			<div class="divSubBlock">
				<div>
					<div class="divValue">LAT<input id="divGpsLat" value="-"></div>
					<div class="divValue">LON<input id="divGpsLon" value="-"></div>
					<div class="divValue">SPD<input id="divGpsSpd" value="-"></div>
					<div class="divValue">ALT<input id="divGpsAlt" value="-"></div>
					<div class="divValue">ANG<input id="divOriAng" value="-"></div>
					<div class="divValue">TIL<input id="divOriTil" value="-"></div>
					<div class="divValue">TEM<input id="divEnvTem" value="-"></div>
					<div class="divValue">HUM<input id="divEnvHum" value="-"></div>
					<div class="divValue">PRS<input id="divEnvPrs" value="-"></div>
					<div class="divValue">LUX<input id="divLuxVal" value="-"></div>
					<div class="divValue" style="display:none;">GYR<input id="divGyrVal" value="-"></div>
					<div class="divValue" style="display:none;">ACL<input id="divAclVal" value="-"></div>
				</div>
				<div>
					<div id="divCompass">
						<div>N</div><div>W</div><div>S</div><div>E</div><div></div>
					</div>
					<div id="divTilt"></div>
				</div>
			</div>
		</div>

		<div id="divPayloads" class="divBlock">
			<span class="title">Payloads</span>
			<div class="divSubBlock">
			</div>
		</div>

		<div id="divScreen" class="divBlock">
			<span class="title">Screen</span>
			<div class="divSubBlock">
				<div id="divScreenProgress"></div>
				<canvas id="cvsScreen" width="240" height="320"></canvas>
				<div id="divScreenCtrl">
					<div id="btnX">&#8617;</div><div id="btnU">&#8593;</div><div id="btnY">&#8620;</div>
					<div id="btnL">&#8592;</div><div id="btnC">&#9737;</div><div id="btnR">&#8594;</div>
					<div id="btnCW">&#10558;</div><div id="btnD">&#8595;</div><div id="btnCCW">&#10559;</div>
				</div>
			</div>
		</div>

		<div id="divConsole" class="divBlock">
			<span class="title">Console</span>
			<div class="divSubBlock">
				<textarea id="wsOutput"></textarea>
				<div class="divValue">CMD<input id="wsInput" value=""></div>
			</div>
		</div>

		<div id="divFBrowser" class="divBlock">
			<span class="title">File Browser</span>
			<div class="divSubBlock">
				<div id="divFiles"></div>
				<div class="divFPanel">
					<div id="divFInfo"></div>
					<div id="divFActions">
						<div id="btnDL">&#10515;</div><div id="btnUL">&#10514;</div><div id="btnED">&#x270E;</div><div id="btnRM">&#x2715;</div><div id="btnYES">&#9760;</div>
					</div>
				</div>
			</div>
		</div>
	</div>
</body>
)=====";