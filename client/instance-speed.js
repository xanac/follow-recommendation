/* Follow recommendation */


window.addEventListener ('load', function () {
	var request = new XMLHttpRequest;
	request.open ('GET', '/cgi-bin/distsn-instance-speed-api.cgi');
	request.onload = function () {
		if (request.readyState === request.DONE) {
			if (request.status === 200) {
				var response_text = request.responseText;
				var instances = JSON.parse (response_text);
				show_instances (instances);
			}
		}
	}
	request.send ();
}, false); /* window.addEventListener ('load', function () { */


function escapeHtml (text) {
        text = text.replace (/\&/g, '&amp;');
        text = text.replace (/\</g, '&lt;');
        text = text.replace (/\>/g, '&gt;');
        return text;
};


function show_instances (instances) {
var placeholder = document.getElementById ('placeholder');
var html = '';
var cn;
for (cn = 0; cn < instances.length; cn ++) {
	var instance;
	instance = instances [cn];
	var instance_html =
		'<p>' +
		'<a href="' +
		'https://' + instance.domain + '" ' +
		'target="distsn-external-instance">' +
		(instance.title? escapeHtml (instance.title): instance.domain) +
		'</a>' +
		'<br>' +
		(instance.speed * 60 * 24).toFixed (1) + ' トゥート/時 (' + (cn + 1) + ' 位)' +
		'</p>';
	html += instance_html;
}
placeholder.innerHTML = html;
} /* function show_users (users) { */


