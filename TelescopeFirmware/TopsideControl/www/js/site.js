$( document ).ready(function() {
	$( 'div.declination input:button' ).click( function(e) {
		var jog_value = $(e.currentTarget).attr('jog_value');
		jog(jog_value);

		ajaxPost("/api/jog/dec", { jog_value: jog_value });
	});

	$( 'div.rightAscension input:button' ).click( function(e) {
		var jog_value = $(e.currentTarget).attr('jog_value');
		jog(jog_value);

		ajaxPost("/api/jog/ra", { jog_value: jog_value });
	});
});

function jog(value) {
	console.log(value);
}

function ajaxPost(url,json) {
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