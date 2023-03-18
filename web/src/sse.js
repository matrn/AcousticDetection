const SSE_RECREATION_TIMEOUT = 5000;

class SSE {
	/* Server Sent Events */

	constructor(angle_callback, log_obj, status_obj, host = window.location.host) {
		this.angle_callback = angle_callback;
		this.log = log_obj;
		this.status = status_obj;

		this.host = host;
		this.url = this.host + '/events';

		this.was_connected = false;

		this._create_sse();
	}

	_create_sse() {
		this.sse = new EventSource(this.url);

		this.sse.addEventListener('open', function (e) {
			this.was_connected = true;
			this.log.info("SSE Connected");
			this.status.set(this.status.GREEN, "SSE active!");
		}.bind(this), false);

		this.sse.addEventListener('error', function (e) {
			if (e.target.readyState != EventSource.OPEN) {
				if (e.target.readyState == EventSource.CONNECTING) {
					this.log.error("SSE disconnected, reconnecting");
					this.status.set(this.status.RED, "SSE disconnected, reconnecting");
				}
				else if (e.target.readyState == EventSource.CLOSED) {
					this.log.error("SSE closed");
					this.status.set(this.status.RED, "SSE closed");

					// When SSE wasn't connected from this start of this JS (it happens when hosted on PC and not on the ESP32), SSE object is not capable of reconnecting, so we have to do it manually
					//console.log(this.was_connected);
					//if (!this.was_connected) {
					setTimeout(function () {
						this.log.debug("SSE recreation");
						this.sse.close();
						delete this.sse;
						this._create_sse();
					}.bind(this), SSE_RECREATION_TIMEOUT);
					//}
				}
				else {
					this.log.error("SSE error");
					this.status.set(this.status.RED, "SSE error");
				}
			}
		}.bind(this), false);

		this.sse.addEventListener('message', function (e) {
			this.log.debug("SSE message: " + e.data);
		}.bind(this), false);

		this.sse.addEventListener('angle', function (e) {
			this.log.debug("SSE ANGLE data: " + e.data);
			this.angle_callback(e.data);
		}.bind(this), false);


		this.log.info("SSE: Trying to connect to " + this.url);
		this.status.set(this.status.YELLOW, 'SSE: Trying to connect');
	}
}