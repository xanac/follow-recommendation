/* Follow recommendation */


window.addEventListener ('load', function () {
	var request = new XMLHttpRequest;
	request.open ('GET', '/cgi-bin/distsn-user-recommendation-api.cgi');
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
		'<a href="' +
		'https://' + user.host + '/@' + user.username +
		'" target="distsn-external-user-profile">' +
		user.username + '@<wbr>' + user.host +
		'</a>' +
		'<br>' +
		(user.speed * 60 * 60 * 24).toFixed (1) + ' トゥート/日 (' + (user.speed_order + 1) + ' 位)' +
		'<br>' +
		'手動得点 ' + (user.manual_score_available? user.manual_score.toFixed (1): '?') + '&emsp;' +
		'総合順位 ' + (cn + 1) + ' 位'
		'</p>';
	html += user_html;
}
placeholder.innerHTML = html;
} /* function show_users (users) { */


