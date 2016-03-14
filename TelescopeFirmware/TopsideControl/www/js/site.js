$( document ).ready(function() {
	$( 'div.declination input:button' ).mousedown( function(e) {
		var jog_value = $(e.currentTarget).attr('jog_value');
		console.log(jog_value);
		ajaxPost("/api/jog/dec", { jog_value: jog_value });
	});

	$( 'div.rightAscension input:button' ).mousedown( function(e) {
		var jog_value = $(e.currentTarget).attr('jog_value');
		console.log(jog_value);
		ajaxPost("/api/jog/ra", { jog_value: jog_value });
	});

	$( 'div.declination input:button' ).mouseup( function(e) {
		ajaxPost("/api/jog/dec", { jog_value: 0 });
	});

	$( 'div.rightAscension input:button' ).mouseup( function(e) {
		ajaxPost("/api/jog/ra", { jog_value: 0 });
	});

	$( 'div.goto input:button' ).click( function(e) {
		var gv = $('#gotoValue').val();
		console.log(gv);
		ajaxPost("/api/goto", { xpos: 0, ypos:gv });
	});
});

function ajaxPost(url,json) {
	console.log(url);
	$.ajax({
		type: "POST",
		url: url,
		// The key needs to match your method's input parameter (case-sensitive).
		data: JSON.stringify(json),
		contentType: "application/json; charset=utf-8",
		dataType: "json",

		success: function(data){console.log(data);},
		failure: function(errMsg) {
			console.log(errMsg);
			alert(errMsg);
		}
	});
}