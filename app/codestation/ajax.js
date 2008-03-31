function xmlHttpPost( url, action, subject ) {
	var xmlHttpReq = false;
	var self = this;
	if ( window.XMLHttpRequest ) // Mozilla/Safari
		self.xmlHttpReq = new XMLHttpRequest();
	else if ( window.ActiveXObject ) // IE
		self.xmlHttpReq = new ActiveXObject( "MicroSuck.XMLHTTP" );
	else {
		alert( "Your browser does not support AJAX." );
		return;
	}
	self.xmlHttpReq.open( 'POST', url, true );
	self.xmlHttpReq.setRequestHeader( 'Content-Type', 'application/x-www-form-urlencoded' );
	self.xmlHttpReq.onreadystatechange = function() {
		try {
			if ( self.xmlHttpReq.readyState == 4 ) {
				if ( self.xmlHttpReq.status != 200 ) {
					alert( "Your request encountered some problems." );
				} else {
//					alert( "ALERT: " + self.xmlHttpReq.statusText );
						update_page( action, self.xmlHttpReq.responseText );
				}
			}
		} catch ( e ) {
			alert ( "Exception: " + e );
		}
	}
//	alert( "ok: " + get_query_string() );
	if ( action != "clear_note" ) {
		self.xmlHttpReq.send( get_query_string( action, subject ) );
	} else {
		document.getElementById( "note" ).innerHTML = "";
	}
}

function get_query_string( action, subject ) {
	switch ( action ) {
		case "filter" : {
			var form = document.forms['query'];
			var word = form.query.value;
			qstr = 'application=codestation&action=ajax&type=query&query=' + escape( word );  // NOTE: no '?' before querystring
			break;
		}
		case "note" : {
			qstr = 'application=codestation&action=ajax&type=note&note=' + subject;
			break;
		}
	}
	return qstr;
}

function update_page( action, str ) {
	switch ( action ) {
		case "filter" : {
			document.getElementById( "result" ).innerHTML = str;
			document.getElementById( "note" ).innerHTML = "";
			break;
		}
		case "note" : {
			document.getElementById( "note" ).innerHTML = str;
			break;
		}
	}
}

