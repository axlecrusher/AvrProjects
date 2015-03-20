var net = require('net');
var EventEmitter = require('events').EventEmitter;
//var microtime = require('microtime');
var util = require( "util" );

function stellarium()
{
	var self = this;

	self.lastPos = {ra:0,dec:0};

	self.listen = function() {
		self.server = net.createServer(function(socket) {
			console.log('client connected');

			socket.write(self.makePositionPacket(self.lastPos));

			self.on('UpdatePosition', function(raw) {
				socket.write(raw);
			});

			socket.on('end', function() {
				console.log('client disconnected');
			});

			socket.on('data', function(raw) {
				var r = parseRaw(raw);
				if (r!=null) {
					console.log(r);
					self.emit('goto', {ra:r.ra,dec:r.dec} );
				}
			});
		});

		self.server.listen(7800,function() {
			console.log('listening');
		});
	}

	self.updatePosition = function(pos) {
		if (JSON.stringify(pos) === JSON.stringify(self.lastPos)) return;
		self.lastPos = pos;

		var raw = self.makePositionPacket(pos);
		self.emit('UpdatePosition', raw);
	}

	self.makePositionPacket = function(pos) {
		var raw = new Buffer(24);
		var ra = Math.floor(pos.ra * (0x80000000 / Math.PI)); //radians to stellarium
		var dec = Math.floor(pos.dec * (0x80000000 / Math.PI));
		var time = Date.now() * 1000;

		console.log(pos);
//		console.log(time);

		raw.writeUInt16LE(24,0);
		raw.writeUInt16LE(0,2);

		writeUInt64LE(raw,time,4);

		raw.writeUInt32LE(ra,12);
		raw.writeInt32LE(dec,16);
		raw.writeInt32LE(0,20);
		return raw;
	}
}

function writeUInt64LE(buffer,value,offset) {
	buffer.writeInt32LE(value&0xFFFFFFFF, 4); //write the low order bits, javascript convertes to 32bit signed
	buffer.writeUInt32LE(Math.floor(value / 0xFFFFFFFF), 8); //high order	
}

function readUInt64LE(buffer,offset) {
	return buffer.readUInt32LE(4) + (buffer.readUInt32LE(8) * 0xFFFFFFFF);  //LSB first
}

function parseRaw(raw) {
	if (raw.readUInt16LE(0) != raw.length)
	{
		console.error('invalid packet size');
		return null;
	}

	var r = {
		length:raw.readUInt16LE(0),
		type:raw.readUInt16LE(2),
		time:readUInt64LE(raw,4),
		ra:raw.readUInt32LE(12),
		dec:raw.readInt32LE(16)
	}

	console.log(r.time);

	if (r.ra<0 || r.ra>0x100000000) {
		console.error('invalid ra:'+r.ra);
		return null;		
	}

	if (r.dec<-0x40000000 || r.dec>0x40000000) {
		console.error('invalid dec:'+r.dec);
		return null;		
	}

	r.ra *= (Math.PI / 0x80000000); //convert to radians
	r.dec *= (Math.PI / 0x80000000);

	return r;
}


util.inherits(stellarium, EventEmitter);
module.exports = stellarium;