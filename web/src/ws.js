class WS {
	constructor(angle_callback, log_obj, status_obj, host = window.location.host) {
		this.angle_callback = angle_callback;
		this.log = log_obj;
		this.status = status_obj;

		this.protocol = 'ws://';
		if (location.protocol == 'https:') this.protocol = 'wss://';
		this.host = host;
		this.url = this.protocol + this.host + '/ws';
	}

	connect() {
		this.log.info("WS: Trying to connect to " + this.url);
		this.status.set(this.status.YELLOW, 'WS: Trying to connect');

		this.socket = new WebSocket(this.url);

		this.socket.onmessage = function (e) {
			this.log.debug("WS: " + e.data);
			this.angle_callback(e.data);
		}.bind(this);

		this.socket.onopen = function (e) {
			this.log.info("WS: open now.");
			this.status.set(this.status.GREEN, 'WS active!');
		}.bind(this);

		this.socket.onclose = function (e) {
			this.log.error('WS: socket closed unexpectedly');
			this.status.set(this.status.RED, 'WS closed, reconnecting');

			setTimeout(function () {
				this.connect();
			}.bind(this), 1000);
		}.bind(this);

		this.socket.onerror = function (event) {
			this.log.error("WS error observed: " + event.type);
			console.log(event);
			this.status.set(this.status.RED, 'WS error: ' + event.type);
		}.bind(this);
	}
}