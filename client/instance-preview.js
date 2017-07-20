window.addEventListener ('load', function () {
	var domain = window.location.search.replace (/^\?/, '');
	if (! domain) {
		var responce = prompt ('ドメイン名を入力してください。');
		if (responce) {
			window.location.search = '?' + responce;
		}
	};
	get_instance (domain);
	get_timeline (domain);
}, false);


function get_instance (domain) {
	var url ='https://' + domain + '/api/v1/instance';
	var request = new XMLHttpRequest ();
	request.onreadystatechange = function () {
		if (request.status == 200 && request.readyState == 4) {
			show_instance (request.response);
		}
	};
	request.responseType = 'json';
	request.open ('GET', url, true);
	request.send ();
}


function show_instance (responce) {
	var html = '';
	html += '<h1>';
	html += '<a href="https://' + responce.uri + '" target="_blank">';
	html += escapeHtml (responce.title);
	html += '</a>';
	html += '</h1>';
	html += responce.description;
	var placeholder = document.getElementById ('placeholder-instance');
	placeholder.innerHTML = html;
};


function get_timeline (domain) {
	var url ='https://' + domain + '/api/v1/timelines/public?local=true&limit=40';
	var request = new XMLHttpRequest ();
	request.onreadystatechange = function () {
		if (request.status == 200 && request.readyState == 4) {
			show (request.response);
		}
	};
	request.responseType = 'json';
	request.open ('GET', url, true);
	request.send ();
}


function show (response) {
	var html;
	var cn;
	html = '';
	for (cn = 0; cn < response.length; cn ++) {
		var toot = response [cn];
		html += show_toot (toot);
	}
	var placeholder = document.getElementById ('placeholder-toots');
	placeholder.innerHTML = html;
};


function show_toot (toot) {
	var html = '';
	html += '<div class="toot">';
	html += '<a href="' + toot.account.url + '" target="_blank">';
	html += '<img src="' + toot.account.avatar + '" width="40" height="40">';
	html += escapeHtml (toot.account.display_name);
	html += '</a>';
	html += '&emsp;';
	html += '<small>';
	html += '<a href="' + toot.url + '" target="_blank">';
	html += (new Date (toot.created_at));
	html += '</a>';
	html += '</small>';
	html += toot.content;
	var attachments = toot.media_attachments;
	if (attachments && 0 < attachments.length) {
		var cn;
		for (cn = 0; cn < attachments.length; cn ++) {
			var attachment = attachments [cn];
			if (attachment.type === 'image') {
				html += '<a href="' + (attachment.remote_url? attachment.remote_url: attachment.url) + '" target="_blank">';
				html += '<img class="preview" src="' + attachment.preview_url + '">';
				html += '</a>';
				html += ' ';
			}
		}
	}
	html += '</div>';
	return html;
};


function escapeHtml (text) {
		text = text.replace (/\&/g, '&amp;');
		text = text.replace (/\</g, '&lt;');
		text = text.replace (/\>/g, '&gt;');
		return text;
};

