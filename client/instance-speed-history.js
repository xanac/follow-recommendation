
function getAndShowFiles () {
var request = new XMLHttpRequest;
request.open ('GET', '/cgi-bin/distsn-instance-speed-history-files-api.cgi');
request.onload = function () {
	if (request.readyState === request.DONE) {
		if (request.status === 200) {
			var responce_text = request.responseText;
			var file_names = responce_text.split ("\n");
			var html = '';
			for (var cn = 0; cn < file_names.length; cn ++) {
				var file_name = file_names [cn];
				var domain = file_name.replace (/\.csv$/, '');
				if (0 < file_name.length) {
					html += '<p><a href="/cgi-bin/distsn-instance-speed-history-api.cgi?' +
						file_name + '" ' +
						'download="' + file_name + '"' +
						'>' +
						domain + '</a></p>';
				}
			}
			document.getElementById ('placeholder').innerHTML = html;
		}
	}
} /* request.onload = function () { */
request.send ();
} /* function getAndShowImages () { */


window.addEventListener ('load', function () {
getAndShowFiles ();
}, false); /* window.addEventListener ('load', function () { */


