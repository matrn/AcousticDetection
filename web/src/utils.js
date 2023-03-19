class Log {
	DEBUG = 'DEBUG';
	INFO = 'INFO';
	WARNING = 'WARNING';
	ERROR = 'ERROR';

	constructor(log_elm) {
		this.log_elm = log_elm;
	}

	_log(d, level) {
		d = level + ": " + new Date().toLocaleTimeString() + ": " + d;
		console.log(d);

		this.log_elm.innerHTML = this.log_elm.innerHTML + d + "<br>";
		this.log_elm.scrollTo(0, this.log_elm.scrollHeight);
	}

	debug(d) {
		this._log(d, this.DEBUG);
	}
	info(d) {
		this._log(d, this.INFO);
	}
	warning(d) {
		this._log(d, this.WARNING);
	}
	error(d) {
		this._log(d, this.ERROR);
	}
}


class Status {
	GREEN = '#28a745';
	RED = '#dc3545';
	YELLOW = '#ffc107';
	BLACK = '#000';

	constructor(status_dot_elm, status_text_elm) {
		this.status_dot = status_dot_elm;
		this.status_text = status_text_elm;
	}

	set(color, text) {
		//console.log("STATUS", color, text, this.status_dot, this.status_text);
		this.status_dot.style.color = color;
		this.status_text.innerHTML = text;
	}
}


class AliveTimer {
	REPEAT = 1000;   // every second
	ALIVE_THRESHOLD = 12 * 1000;   // 12 seconds
	expired_callback = undefined;

	constructor(elm) {
		this.elm = elm;
		this.interval = undefined;
		this.date = undefined;
		this.outdated = false;
		this.expired_callback_called = false;
	}

	new_alive(date) {
		this.date = date;
		this.outdated = false;
		this.expired_callback_called = false;

		if (!this.interval) {
			this.interval = setInterval(function () {
				let curr = new Date();
				let diff_ms = curr.getTime() - this.date.getTime();

				if (diff_ms > this.ALIVE_THRESHOLD) {
					this.outdated = true;
					if (this.expired_callback && !this.expired_callback_called) {
						this.expired_callback_called = true;
						this.expired_callback();
					}
				}
				this.elm.innerHTML = `${Math.round(diff_ms / 1000)}s ago${this.outdated ? ' ❌' : ''}`;
			}.bind(this), this.REPEAT);
		}
	}
}