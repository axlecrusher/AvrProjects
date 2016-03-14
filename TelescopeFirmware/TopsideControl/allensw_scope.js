var EventEmitter = require('events').EventEmitter;
var util = require( "util" );
var usb = require('usb')

function AllenswScope()
{
	var self = this;

	this.ra = 0;
	this.dec = 0;

	this.dest_ra = 0;
	this.dest_dec = 0;

	self.goto = function(ra,dec) {
		var self = this;
		self.dest_ra = ra;
		self.dest_dec = dec;

		console.log(ra);

		self.sendYdest(self.dest_ra);
	} 

	self.jog = function(ra,dec) {
		var self = this;

		var buffer = new Buffer(2);
		buffer.writeInt8(ra, 0);
		buffer.writeInt8(dec, 1);
		console.log(buffer);

		if (self.device == null) return;

		self.device.controlTransfer(usb.LIBUSB_ENDPOINT_OUT | usb.LIBUSB_REQUEST_TYPE_VENDOR | usb.LIBUSB_RECIPIENT_DEVICE, //reqtype
			0xA8, //request
			0x0100, //wValue
			0x0000, //wIndex
			buffer,
			function(err,data) {
				if(err) console.log(err);
				else
				console.log('sent ydest');
		});
	} 

	self.getInfo = function() {
		var self = this;
		return {ra:self.ra,dec:self.dec};
	}

	self.slew = function() {
		var self = this;
		if (self.ra > self.dest_ra) self.ra -= Math.min(slewStep,self.ra-self.dest_ra);
		if (self.ra < self.dest_ra) self.ra += Math.min(slewStep,self.dest_ra-self.ra);
		if (self.dec > self.dest_dec) self.dec -= Math.min(slewStep,self.dec-self.dest_dec);
		if (self.dec < self.dest_dec) self.dec += Math.min(slewStep,self.dest_dec-self.dec);
	}


	self.findUsbDevice = function() {
//		console.log('looking for device');
		var device = usb.findByIds(0xabcd, 0xf013);
		if (device) {
			console.log(device);
			device.open();
			self.device = device;
			self.sendPing(function(err) {
				if (err) { console.log('error doing ping'); }
				self.emit('foundDevice');
				self.startMotors();
			});
		}
//		else
//			console.log('not found');
	}

	self.sendPing = function(clbk) {
		self.device.controlTransfer(usb.LIBUSB_ENDPOINT_IN | usb.LIBUSB_REQUEST_TYPE_VENDOR | usb.LIBUSB_RECIPIENT_DEVICE, 
			0xA1, 0x0100, 0x0000, 16, function(err, data) {
				if (!err) console.log(data.toString('ascii'));
				clbk(err);
		});		
	}

	self.startMotors = function() {
		if (self.device == null) return;
		self.device.controlTransfer(usb.LIBUSB_ENDPOINT_IN|usb.LIBUSB_REQUEST_TYPE_VENDOR|usb.LIBUSB_RECIPIENT_DEVICE,
			0xA4,
			0x0100, //wValue
			0x0000, //wIndex
			16,
			function(err,data) {
				if(err) console.error(err);
				else console.log(data.toString('ascii'));
		});
	}

	self.sendYdest = function(dec) {
		var buffer = new Buffer(4);
		buffer.writeUInt32LE(dec);
		console.log(buffer);
		if (self.device == null) return;
		self.device.controlTransfer(usb.LIBUSB_ENDPOINT_OUT | usb.LIBUSB_REQUEST_TYPE_VENDOR | usb.LIBUSB_RECIPIENT_DEVICE, //reqtype
			0xA6, //request
			0x0100, //wValue
			0x0000, //wIndex
			buffer,
			function(err,data) {
				if(err) console.log(err);
				else
				console.log('sent ydest');
		});
	}

	self.get_motor_info = function() {
		if (self.device == null) return;
//		var buffer = new Buffer(16);
//		console.log(buffer);
		if (self.device == null) return;
		self.device.controlTransfer(usb.LIBUSB_ENDPOINT_IN | usb.LIBUSB_REQUEST_TYPE_VENDOR | usb.LIBUSB_RECIPIENT_DEVICE | 0x88, //reqtype
			0xA3, //request
			0x0100, //wValue
			0x0000, //wIndex
			16,
			function(err,data) {
				if(err) console.log(err);
				else {
					/*
					//XXXXXX
					console.log(data);
					console.log(data.readUInt32LE(0));
					console.log(data.readInt32LE(4));
					console.log(data.readUInt32LE(8));
					*/
//					console.log(data.readInt32LE(12));
//					console.log(data + ' motor ' + x);
				}
		});
	}

	self.setupDevicePoll = function() {
		setInterval(function() {
			if (self.device) {
				self.sendPing(function(err) {
					if (err) {
						//lost device.
						console.error('lost usb device');
						self.emit('lostDevice');
						self.device = undefined;
					}
				});
			}
			else
			{
				self.findUsbDevice();
			}
		},1000);
	}

	self.setupDevicePoll();

//	self.findUsbDevice();
//	self.startMotors();

}

util.inherits(AllenswScope, EventEmitter);
module.exports = AllenswScope;