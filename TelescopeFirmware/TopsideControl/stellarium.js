var net = require('net');
var EventEmitter = require('events').EventEmitter;
//var microtime = require('microtime');
var util = require( "util" );

function stellarium()
{
	var self = this;

	self.listen = function() {
		self.server = net.createServer(function(socket) {
			console.log('client connected');
			self.on('CurrentPosition', function(raw) {
				socket.write(raw);;
			});

			socket.on('end', function() {
				console.log('client disconnected');
			});

			socket.on('data', function(raw) {
				var r = parseRaw(raw);
				if (r!=null) {
					console.log(r);
					self.emit('goto',{r.ra,r.dec});
				}
			});
		});

		self.server.listen(8124,function() {
			console.log('listening');
		});
	}

	self.updatePosition = function(pos) {
		var raw;
		pos.ra *= (0x80000000 / Math.PI); //radians to stellarium
		pos.dec *= (0x80000000 / Math.PI);
		self.emit('CurrentPosition', raw);
	}
}

function parseRaw(raw) {
	if (raw.readUInt16LE(0) != raw.length)
	{
		consle.error('invalid packet size');
		return null;
	}

	var r = {
		length:raw.readUInt16LE(0),
		type:raw.readUInt16LE(2),
		time:raw.readUInt64LE(4),
		ra:raw.readUInt64LE(12),
		dec:raw.raw.readUInt64LE(16)
	}

	if (r.ra<0 || r.ra>0x100000000) {
		consle.error('invalid ra');
		return null;		
	}

	if (r.ra<-0x40000000 || r.ra>0x40000000) {
		consle.error('invalid dec');
		return null;		
	}

	r.ra *= (Math.PI / 0x80000000); //convert to radians
	r.dec *= (Math.PI / 0x80000000);

	return r;
}


util.inherits(stellarium, EventEmitter);
module.exports = stellarium;