var net = require('net');
var EventEmitter = require('events').EventEmitter;
//var microtime = require('microtime');
var util = require( "util" );

var slewStep = 0.00628318531;

function dumb_scope()
{
	var self = this;

	self.ra = 0;
	self.dec = 0;

	self.dest_ra = 0;
	self.dest_dec = 0;

	setInterval(function() { self.slew(); }, 100);

	self.goto = function(ra,dec) {
		self.dest_ra = ra;
		self.dest_dec = dec;
	}

	self.getInfo = function() {
		return {ra:self.ra,dec:self.dec};
	}

	self.slew = function() {
		if (self.ra > self.dest_ra) self.ra -= Math.min(slewStep,self.ra-self.dest_ra);
		if (self.ra < self.dest_ra) self.ra += Math.min(slewStep,self.dest_ra-self.ra);
		if (self.dec > self.dest_dec) self.dec -= Math.min(slewStep,self.dec-self.dest_dec);
		if (self.dec < self.dest_dec) self.dec += Math.min(slewStep,self.dest_dec-self.dec);
	}
}

util.inherits(dumb_scope, EventEmitter);
module.exports = dumb_scope;