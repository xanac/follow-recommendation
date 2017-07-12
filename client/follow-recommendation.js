/* Follow recommendation */


window.addEventListener ('load', function () {
	var request = new XMLHttpRequest;
	request.open ('GET', '/cgi-bin/distsn-user-speed-api.cgi');
	request.onload = function () {
		if (request.readyState === request.DONE) {
			if (request.status === 200) {
				var response_text = request.responseText;
				var users = JSON.parse (response_text);
				show_users (users);
			}
		}
	}
	request.send ();
}, false); /* window.addEventListener ('load', function () { */


function show_users (users) {
var placeholder = document.getElementById ('placeholder-users');
var html = '';
var cn;
for (cn = 0; cn < users.length; cn ++) {
	var user;
	user = users [cn];
	var user_html =
		'<p>' +
		user.host + ' ' +
		user.username + ' ' +
		user.speed +
		'</p>';
	html += user_html;
}
placeholder.innerHTML = html;
} /* function show_users (users) { */


