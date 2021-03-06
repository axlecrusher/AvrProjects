var	express = require('express'),
	app = express(),
	apiRoute = express.Router(),
	bodyParser = require('body-parser');

var async = require('async');
var underscore = require('underscore');

var stellarium = require('./stellarium.js');
var s = new stellarium();

var telescope = require('./allensw_scope.js');
var scope = new telescope();

s.listen();

setInterval(function() {
	var r = scope.getInfo();
//	console.log(r);
	scope.get_motor_info();
	s.updatePosition(r);
}, 100);

s.on('goto',function(pos){
	scope.goto(pos.ra,pos.dec);
})

app.listen(8080);

app.use('/', express.static(__dirname + '/www'));
app.use(bodyParser.json());
app.use('/api', function (req, res, next) {
		console.log(req.originalUrl);
		next();
	},
	apiRoute
);

apiRoute.route('/info').get(function(request, response) {
	console.log(request.body);
	response.end(JSON.stringify({status:'ok'}));
});

apiRoute.route('/jog/:axis').post(function(request, response) {
//	console.log(request);
//	console.log(request.params.axis + ':' + request.body.jog_value);
	var data = {ra:0,dec:0};
	if (request.params.axis === 'ra') data.ra=request.body.jog_value;
	if (request.params.axis === 'dec') data.dec=request.body.jog_value;
	response.end(JSON.stringify({status:'ok'}));
	console.log(data);
	scope.jog(data.ra,data.dec);
});

apiRoute.route('/goto').post(function(request, response) {
	var data = request.body;
	console.log(data);
	response.end(JSON.stringify({status:'slewing'}));
	scope.goto(data.ypos,0);
});