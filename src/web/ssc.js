// ==UserScript==
// @match *://www.skyscrapercity.com/*
// @run-at document-end
// ==/UserScript==

(function(){
	var replyDialog

	console.log("Connecting to Fotorelacjonusz…")
	initWebChannel()

	/**
	 * Waits till QWebChannel constructor is available (script load order is
	 * quite random), and then creates a new instance, passing channelReady
	 * function as a callback.
	 */
	function initWebChannel() {
		if (typeof(QWebChannel) === "function") {
			new QWebChannel(qt.webChannelTransport, channelReady)
		} else {
			setTimeout(initWebChannel, 50)
		}
	}

	function channelReady(channel) {
		console.log("Established connection to Fotorelacjonusz.")

		replyDialog = channel.objects.replyDialog
		replyDialog.forumPageLoaded(location.href)
	}
})()
